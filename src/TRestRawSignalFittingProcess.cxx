/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
///
/// Fit every TRestRawSignal in a TRestRawSignalEvent with AGET theoretical
/// curve times a logistic function. This logistic function acts like a step function
/// to select only the positive range of the AGET function.
/// Working with raw signal (without subtracting baseline).
///
/// From TRestRawSignal -> TH1 -> Measure goodness of fit
///
/// Analytic expression to fit:
///
/// \code
/// [0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * (x-[3])/[2] * (x-[3])/[2] *
/// (x-[3])/[2] * sin((x-[3])/[2])/(1+TMath::Exp(-x+[3]))
///
/// [0] = "Baseline",
/// [1] = "Amplitude",
/// [2] = "ShapingTime",
/// [3] = "PeakPosition"
/// \endcode
///
/// \htmlonly <style>div.image img[src="Fit600.png"]{width:500px;}</style>
/// \endhtmlonly
///
/// ![Example of fitted pulse](Fit600.png)
///
/// ### Observables
///
/// * **FitBaseline_map**: For each pulse, save first fit's parameter.
///
/// * **FitAmplitude_map**: For each pulse, save second fit's parameter.
///
/// * **FitShapingTime_map**: For each pulse, save third fit's parameter.
///
/// * **FitPeakPosition_map**: For each pulse, save fourth fit's parameter.
///
/// * **FitSigmaMean**: Mean over all pulses in the event of square root of the
/// squared
/// difference between raw signal and fit divided by number of bins.
///
/// * **FitSigmaStdDev**: Standard deviation over all pulses in the event of
/// square root of the squared
/// difference between raw signal and fit divided by number of bins.
///
/// * **FitChiSquareMean**: Mean over all pulses in the event of chi square of
/// the fit.
///
/// * **FitRatioSigmaMaxPeakMean**: Mean over all pulses in the event of square
/// root of the squared
/// difference between raw signal and fit divided by number of bins and divided
/// by amplitude of the pulse.
///
/// <hr>
///
/// \warning **⚠ REST is under continuous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code up to date. Your feedback will be worth to support this software, please
/// report any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///_______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2020-August First implementation of raw signal fitting process.
///                Created from TRestRawSignalAnalysisProcess.
///
/// \class      TRestRawSignalFittingProcess
/// \author     David Diez
///
/// <hr>
///
#include "TRestRawSignalFittingProcess.h"

using namespace std;

ClassImp(TRestRawSignalFittingProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalFittingProcess::TRestRawSignalFittingProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// The default behaviour is that the config file must be specified with
/// full path, absolute or relative.
///
/// \param configFilename A const char* giving the path to an RML file.
///
TRestRawSignalFittingProcess::TRestRawSignalFittingProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalFittingProcess::~TRestRawSignalFittingProcess() {}

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawSignalFittingProcess::LoadDefaultConfig() { SetTitle("Default config"); }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalFittingProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fRawSignalEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Function to load the configuration from an external configuration
/// file.
///
/// If no configuration path is defined in TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// \param configFilename A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the
/// corresponding TRestGeant4AnalysisProcess section inside the RML.
///
void TRestRawSignalFittingProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalFittingProcess::InitProcess() {
    // fSignalFittingObservables = TRestEventProcess::ReadObservables();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalFittingProcess::ProcessEvent(TRestEvent* inputEvent) {
    // no need for verbose copy now
    fRawSignalEvent = (TRestRawSignalEvent*)inputEvent;

    debug << "TRestRawSignalFittingProcess::ProcessEvent. Event ID : " << fRawSignalEvent->GetID() << endl;

    Double_t SigmaMean = 0;
    Double_t Sigma[fRawSignalEvent->GetNumberOfSignals()];
    Double_t RatioSigmaMaxPeakMean = 0;
    Double_t RatioSigmaMaxPeak[fRawSignalEvent->GetNumberOfSignals()];
    Double_t ChiSquareMean = 0;
    Double_t ChiSquare[fRawSignalEvent->GetNumberOfSignals()];

    map<int, Double_t> baselineFit;
    map<int, Double_t> amplitudeFit;
    map<int, Double_t> shapingTimeFit;
    map<int, Double_t> peakPositionFit;

    baselineFit.clear();
    amplitudeFit.clear();
    shapingTimeFit.clear();
    peakPositionFit.clear();

    for (int s = 0; s < fRawSignalEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* singleSignal = fRawSignalEvent->GetSignal(s);

        int MaxPeakBin = singleSignal->GetMaxPeakBin();

        // ShaperSin function (AGET theoretic curve) times logistic function
        TF1* f = new TF1("fit",
                         "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                         "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] * "
                         "sin((x-[3])/[2])/(1+TMath::Exp(-10000*(x-[3])))",
                         0, 511);
        f->SetParameters(0, 2000, 70, 80);
        // f->SetParameters(0, 0);  // Initial values adjusted from Desmos
        // f->SetParLimits(0, 150, 350);
        // f->SetParameters(1, 2000);
        // f->SetParLimits(1, 30, 90000);
        // f->SetParameters(2, 70);
        // f->SetParLimits(2, 10, 80);
        // f->SetParameters(3, 80);
        // f->SetParLimits(3, 150, 250);
        f->SetParNames("Baseline", "Amplitude", "ShapingTime", "PeakPosition");

        // Create histogram from signal
        Int_t nBins = singleSignal->GetNumberOfPoints();
        TH1D* h = new TH1D("histo", "Signal to histo", nBins, 0, nBins);

        for (int i = 0; i < nBins; i++) {
            h->Fill(i, singleSignal->GetRawData(i));
        }

        // Fit histogram with ShaperSin
        h->Fit(f, "NWW", "", 0, 511);  // Options: R->fit in range, N->No draw, Q->Quiet

        Double_t sigma = 0;
        for (int j = MaxPeakBin - 145; j < MaxPeakBin + 165; j++) {
            sigma += (singleSignal->GetRawData(j) - f->Eval(j)) * (singleSignal->GetRawData(j) - f->Eval(j));
        }
        Sigma[s] = TMath::Sqrt(sigma / (145 + 165));
        RatioSigmaMaxPeak[s] = Sigma[s] / singleSignal->GetRawData(MaxPeakBin);
        RatioSigmaMaxPeakMean += RatioSigmaMaxPeak[s];
        SigmaMean += Sigma[s];
        ChiSquare[s] = f->GetChisquare();
        ChiSquareMean += ChiSquare[s];

        baselineFit[singleSignal->GetID()] = f->GetParameter(0);
        amplitudeFit[singleSignal->GetID()] = f->GetParameter(1);
        shapingTimeFit[singleSignal->GetID()] = f->GetParameter(2);
        peakPositionFit[singleSignal->GetID()] = f->GetParameter(3);

        fShaping = f->GetParameter(2);
        fStartPosition = f->GetParameter(3);
        fBaseline = f->GetParameter(0);
        fAmplitude = f->GetParameter(1);

        h->Delete();
    }

    //////////// Fitted parameters Map Observables /////////////
    SetObservableValue("FitBaseline_map", baselineFit);
    SetObservableValue("FitAmplitude_map", amplitudeFit);
    SetObservableValue("FitShapingTime_map", shapingTimeFit);
    SetObservableValue("FitPeakPosition_map", peakPositionFit);

    //////////// Sigma Mean Observable /////////////
    SigmaMean = SigmaMean / (fRawSignalEvent->GetNumberOfSignals());
    SetObservableValue("FitSigmaMean", SigmaMean);

    //////////// Sigma Mean Standard Deviation  Observable /////////////
    Double_t sigmaMeanStdDev = 0;
    for (int k = 0; k < fRawSignalEvent->GetNumberOfSignals(); k++) {
        sigmaMeanStdDev += (Sigma[k] - SigmaMean) * (Sigma[k] - SigmaMean);
    }
    Double_t SigmaMeanStdDev = TMath::Sqrt(sigmaMeanStdDev / fRawSignalEvent->GetNumberOfSignals());
    SetObservableValue("FitSigmaStdDev", SigmaMeanStdDev);

    //////////// Chi Square Mean Observable /////////////
    ChiSquareMean = ChiSquareMean / fRawSignalEvent->GetNumberOfSignals();
    SetObservableValue("FitChiSquareMean", ChiSquareMean);

    //////////// Ratio Sigma MaxPeak Mean Observable /////////////
    RatioSigmaMaxPeakMean = RatioSigmaMaxPeakMean / fRawSignalEvent->GetNumberOfSignals();
    SetObservableValue("FitRatioSigmaMaxPeakMean", RatioSigmaMaxPeakMean);

    debug << "SigmaMean: " << SigmaMean << endl;
    debug << "SigmaMeanStdDev: " << SigmaMeanStdDev << endl;
    debug << "ChiSquareMean: " << ChiSquareMean << endl;
    debug << "RatioSigmaMaxPeakMean: " << RatioSigmaMaxPeakMean << endl;
    for (int k = 0; k < fRawSignalEvent->GetNumberOfSignals(); k++) {
        debug << "Standard deviation of signal number " << k << ": " << Sigma[k] << endl;
        debug << "Chi square of fit signal number " << k << ": " << ChiSquare[k] << endl;
        debug << "Sandard deviation divided by amplitude of signal number " << k << ": "
              << RatioSigmaMaxPeak[k] << endl;
    }

    /// We define (or re-define) the baseline range and calculation range of our
    /// raw-signals.
    // This will affect the calculation of observables, but not the stored
    // TRestRawSignal data.
    // fRawSignalEvent->SetBaseLineRange(fBaseLineRange);
    // fRawSignalEvent->SetRange(fIntegralRange);

    /* Perhaps we want to identify the points over threshold where to apply the
  fitting?
     * Then, we might need to initialize points over threshold
     *
  for (int s = 0; s < fRawSignalEvent->GetNumberOfSignals(); s++) {
    TRestRawSignal* sgnl = fRawSignalEvent->GetSignal(s);

    /// Important call we need to initialize the points over threshold in a
  TRestRawSignal
    sgnl->InitializePointsOverThreshold(TVector2(fPointThreshold,
  fSignalThreshold),
                                        fNPointsOverThreshold);

  }
    */

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    return fRawSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. This method will write the channels histogram.
///
void TRestRawSignalFittingProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}
