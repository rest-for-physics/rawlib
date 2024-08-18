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
/// Fit every TRestRawSignal in a TRestRawSignalEvent with fuction provided
/// in the RML file.
/// It uses CreateTF1FromString function from TRestStringHelper.
/// Examples:
/// -- Initial value: [0=3.5]
/// -- Fixed value: [0==3.5]
/// -- Range: [0=3.5(1,5)] The parameter 0 begin at 3.5 and it can move between 1 and 5.
///
/// -- Complete function example: [0=0(-100,100)]+[1=2000]*TMath::Exp(-3. * (x-[3=80])/[2=70])
///                               * ((x-[3=80])/[2=70])^3  * sin((x-[3=80])/[2=70])
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
/// 2021-June First implementation of General Fit Process.
///                Created from TRestRawSignalAnalysisProcess.
///
/// \class      TRestRawSignalGeneralFitProcess
/// \author     David Diez
///
/// <hr>
///
#include "TRestRawSignalGeneralFitProcess.h"

using namespace std;

ClassImp(TRestRawSignalGeneralFitProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalGeneralFitProcess::TRestRawSignalGeneralFitProcess() { Initialize(); }

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
TRestRawSignalGeneralFitProcess::TRestRawSignalGeneralFitProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalGeneralFitProcess::~TRestRawSignalGeneralFitProcess() {}

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawSignalGeneralFitProcess::LoadDefaultConfig() { SetTitle("Default config"); }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalGeneralFitProcess::Initialize() {
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
void TRestRawSignalGeneralFitProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalGeneralFitProcess::InitProcess() {
    // fSignalFittingObservables = TRestEventProcess::ReadObservables();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalGeneralFitProcess::ProcessEvent(TRestEvent* inputEvent) {
    // no need for verbose copy now
    fRawSignalEvent = (TRestRawSignalEvent*)inputEvent;

    RESTDebug << "TRestRawSignalGeneralFitProcess::ProcessEvent. Event ID : " << fRawSignalEvent->GetID()
              << RESTendl;

    Double_t SigmaMean = 0;
    vector<Double_t> Sigma(fRawSignalEvent->GetNumberOfSignals());
    Double_t RatioSigmaMaxPeakMean = 0;
    vector<Double_t> RatioSigmaMaxPeak(fRawSignalEvent->GetNumberOfSignals());
    Double_t ChiSquareMean = 0;
    vector<Double_t> ChiSquare(fRawSignalEvent->GetNumberOfSignals());

    /*map<int, Double_t> baselineFit;
    map<int, Double_t> amplitudeFit;
    map<int, Double_t> shapingtimeFit;
    map<int, Double_t> peakpositionFit;

    baselineFit.clear();
    amplitudeFit.clear();
    shapingtimeFit.clear();
    peakpositionFit.clear();*/

    if (fFitFunc) {
        delete fFitFunc;
        fFitFunc = nullptr;
    }
    fFitFunc = CreateTF1FromString(fFunction, fFunctionRange.X(), fFunctionRange.Y());

    std::vector<map<int, Double_t>> param(fFitFunc->GetNpar());
    std::vector<map<int, Double_t>> paramErr(fFitFunc->GetNpar());

    for (int s = 0; s < fRawSignalEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* singleSignal = fRawSignalEvent->GetSignal(s);

        int MaxPeakBin = singleSignal->GetMaxPeakBin();

        /*// ShaperSin function (AGET theoretic curve) times logistic function
        TF1* f = new TF1("fit",
                         "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                         "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] * "
                         "sin((x-[3])/[2])/(1+TMath::Exp(-10000*(x-[3])))",
                         0, 511);
        f->SetParameters(0, 2000, 70, 80);
        //f->SetParameters(0, 0);  // Initial values adjusted from Desmos
        //f->SetParLimits(0, 150, 350);
        //f->SetParameters(1, 2000);
        //f->SetParLimits(1, 30, 90000);
        //f->SetParameters(2, 70);
        //f->SetParLimits(2, 10, 80);
        //f->SetParameters(3, 80);
        //f->SetParLimits(3, 150, 250);
        //f->FixParameter(2, 75);
        f->SetParNames("Baseline", "Amplitude", "ShapingTime", "PeakPosition");*/

        // Create histogram from signal
        Int_t nBins = singleSignal->GetNumberOfPoints();
        TH1D* h = new TH1D("histo", "Signal to histo", nBins, 0, nBins);

        for (int i = 0; i < nBins; i++) {
            h->Fill(i, singleSignal->GetRawData(i));
        }

        // Fit histogram with ShaperSin
        h->Fit(fFitFunc, "NWW", "", 0, 511);  // Options: R->fit in range, N->No draw, Q->Quiet

        Double_t sigma = 0;
        for (int j = fFunctionRange.X(); j < fFunctionRange.Y(); j++) {
            sigma += (singleSignal->GetRawData(j) - fFitFunc->Eval(j)) *
                     (singleSignal->GetRawData(j) - fFitFunc->Eval(j));
        }
        Sigma[s] = TMath::Sqrt(sigma / (fFunctionRange.Y() - fFunctionRange.X()));
        RatioSigmaMaxPeak[s] = Sigma[s] / singleSignal->GetRawData(MaxPeakBin);
        RatioSigmaMaxPeakMean += RatioSigmaMaxPeak[s];
        SigmaMean += Sigma[s];
        ChiSquare[s] = fFitFunc->GetChisquare();
        ChiSquareMean += ChiSquare[s];

        for (int i = 0; i < fFitFunc->GetNpar(); i++) {
            param[i][singleSignal->GetID()] = fFitFunc->GetParameter(i);
            paramErr[i][singleSignal->GetID()] = fFitFunc->GetParError(i);
            RESTDebug << "Parameter " << i << ": " << param[i][singleSignal->GetID()] << RESTendl;
            RESTDebug << "Error parameter " << i << ": " << paramErr[i][singleSignal->GetID()] << RESTendl;
        }

        /*baselineFit[singleSignal->GetID()] = f->GetParameter(0);
        amplitudeFit[singleSignal->GetID()] = f->GetParameter(1);
        shapingtimeFit[singleSignal->GetID()] = f->GetParameter(2);
        peakpositionFit[singleSignal->GetID()] = f->GetParameter(3);

        fShaping = f->GetParameter(2);
        fStartPosition = f->GetParameter(3);
        fBaseline = f->GetParameter(0);
        fAmplitude = f->GetParameter(1);*/

        h->Delete();
    }

    //////////// Fitted parameters Map Observables /////////////
    /*SetObservableValue("FitBaseline_map", baselineFit);
    SetObservableValue("FitAmplitude_map", amplitudeFit);
    SetObservableValue("FitShapingTime_map", shapingtimeFit);
    SetObservableValue("FitPeakPosition_map", peakpositionFit);*/

    //////////// Fitted parameters Map Observables /////////////
    for (int i = 0; i < fFitFunc->GetNpar(); i++) {
        SetObservableValue("Param_" + to_string(i) + "_map", param[i]);
        SetObservableValue("ParamErr_" + to_string(i) + "_map", paramErr[i]);
    }

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

    RESTDebug << "SigmaMean: " << SigmaMean << RESTendl;
    RESTDebug << "SigmaMeanStdDev: " << SigmaMeanStdDev << RESTendl;
    RESTDebug << "ChiSquareMean: " << ChiSquareMean << RESTendl;
    RESTDebug << "RatioSigmaMaxPeakMean: " << RatioSigmaMaxPeakMean << RESTendl;
    for (int k = 0; k < fRawSignalEvent->GetNumberOfSignals(); k++) {
        RESTDebug << "Standard deviation of signal number " << k << ": " << Sigma[k] << RESTendl;
        RESTDebug << "Chi square of fit signal number " << k << ": " << ChiSquare[k] << RESTendl;
        RESTDebug << "Sandard deviation divided by amplitude of signal number " << k << ": "
                  << RatioSigmaMaxPeak[k] << RESTendl;
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
void TRestRawSignalGeneralFitProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
    if (fFitFunc) {
        delete fFitFunc;
        fFitFunc = nullptr;
    }
}
