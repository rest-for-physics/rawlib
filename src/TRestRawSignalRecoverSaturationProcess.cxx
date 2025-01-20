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

/////////////////////////////////////////////////////////////////////////
/// This class is a process to recover the signals that saturated the ADC.
/// The process uses a fit to recover the signal lost information. If the fit
/// is successful, the points of the signal that are saturating are replaced
/// by the fit values at those bins.
/// \htmlonly <style>div.image img[src="RecoverSignalProcess_eventRecovered.png"]{width:350px;}</style> \endhtmlonly
///
/// The idea of the process is to recover the signal information that is lost
/// when the signal saturates the ADC in order to continue the analysis of the
/// event as if the signal was not saturated at all. For example, here is the 
/// comparison in the spectrum of the observable ThresholdIntegral of the
/// TRestRawSignalAnalysisProcess before (red) and after (blue) applying this
/// TRestRawSignalRecoverSaturationProcess:
/// \htmlonly <style>div.image img[src="RecoverSignalProcess_spectrumComparison.png"]{width:350px;}</style> \endhtmlonly
///
/// ### Fitting function
/// The fitting function is fixed (hardcoded) to the AGET response function
/// (without the sin) times a logistic function:
/// \code
/// [0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * 
/// (x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] / 
/// (1+TMath::Exp(-10000*(x-[3])))
///
/// [0] = "Baseline",
/// [1] = "Amplitude",
/// [2] = "ShapingTime",
/// [3] = "PeakPosition"
/// \endcode
/// \htmlonly <style>div.image img[src="RecoverSignalProcess_signalFit.png"]{width:350px;}</style> \endhtmlonly
///
///
/// ### Parameters
/// The default behaviour is to process only the saturated signals, but it
/// can be configured to process all signals in the event
/// (using the fProcessAllSignals parameter). The saturated signals are
/// identified by a minimun number of bins (fMinSaturatedBins) that must 
/// have the maximum value of the signal and by a threshold value which
/// this saturating value must exceed (fMinSaturationValue).
///
/// The fit can be configured with the fBaseLineRange and fFitRange parameters.
/// The baseline range is used to calculate the baseline of the signal and fix
/// that parameter in the fit. This will make the fit faster and more reliable.
/// The fit range is the range of bins to fit the signal. If not provided, the
/// whole signal will be used.
///
/// To debug the process, you may use _debug_ or _extreme_ verbose levels.
/// In the extreme level, a canvas will be created to draw the signal and
/// the fit for each processed signal of the event. In the debug level, the
/// fit result will be printed in the console but no canvas will be created.
///
/// * **minSaturatedBins**: Minimum number of saturated bins to consider a signal as saturated.
/// Default is 3.
/// * **minSaturationValue**: Threshold to consider a maximum value of the signal as possible saturation.
/// Default is 0.
/// * **baseLineRange**: Range of bins to calculate the baseline and fix that parameter in the fit.
/// * **fitRange**: Range of bins to fit the signal.
/// * **processAllSignals**: If false (default), only signals considered as saturated will be processed.
/// If true, all signals will be processed.
/// * **nBinsIfNotSaturated**: Number of bins to consider as 'saturated' if the signal is not saturated.
/// This is used when fProcessAllSignals is true to set the 'saturated' bins.
///
/// ### Observables
/// The process will add the following observables to the event:
/// * **addedAmplitude**: Sum of the extra amplitude (newamplitude - previousamplitude) of the
/// recovered saturated signals of the event.
/// * **addedIntegral**: Sum of the extra integral (sum of newvalue-previousvalue of the saturated bins)
/// of the recovered saturated signals of the event.
/// * **saturatedSignals**: number of signals of the event detected as saturated.
/// * **recoveredSignals**: number of signals of the event detected as saturated and that have been
/// modified by the fitted pulse. It is always <=saturatedSignals. If it is less, it means that
/// the fit values of the saturated bins of the those signals were lower than the original value
/// of the signal so they are not replaced and those signals has not been recovered.
/// \htmlonly <style>div.image img[src="RecoverSignalProcess_addedAmplIntegral.png"]{width:350px;}</style> \endhtmlonly
///
///
/// ### Examples
/// Give examples of usage and RML descriptions that can be tested.
/// \code
///    <addProcess type="TRestRawSignalRecoverSaturationProcess" name="recSat" value="ON" verboseLevel="info" observable="all">
///        <parameter name="minSaturatedBins" value="3" />
///        <parameter name="minSaturationValue" value="3500" />
///        <parameter name="baseLineRange" value="(20,150)" />
///        <parameter name="fitRange" value="(150,300)" />
///    </addProcess>
/// \endcode
///
///
///----------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2025-Jan: First implementation of TRestRawSignalRecoverSaturationProcess
/// Álvaro Ezquerro
///
/// \class TRestRawSignalRecoverSaturationProcess
/// \author: aezquerro
///
/// <hr>
///

#include "TRestRawSignalRecoverSaturationProcess.h"

ClassImp(TRestRawSignalRecoverSaturationProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalRecoverSaturationProcess::TRestRawSignalRecoverSaturationProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalRecoverSaturationProcess::~TRestRawSignalRecoverSaturationProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestRawSignalRecoverSaturationProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
    fAnaEvent = NULL;

    // Initialize here the values of class data members if needed
    fMinSaturatedBins = 3;
    fProcessAllSignals = false;
    fNBinsIfNotSaturated = 20;
    fMinSaturationValue = 0;
    fBaseLineRange = TVector2(-1, -1);  // -1 means no baseline range
    fFitRange = TVector2(-1, -1);       // -1 means no fit range
}

///////////////////////////////////////////////
/// \brief Process initialization. Observable names can be re-interpreted
/// here. Any action in the process required before starting event process
/// might be added here.
///
void TRestRawSignalRecoverSaturationProcess::InitProcess() {
    // Write here the jobs to do before processing
    // i.e., initialize histograms and auxiliary vectors,
    // read TRestRun metadata, or load additional rml sections
    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
        fC = new TCanvas("c", "c", 800, 600);
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalRecoverSaturationProcess::ProcessEvent(TRestEvent* evInput) {
    fAnaEvent = (TRestRawSignalEvent*)evInput;
    auto eventID = fAnaEvent->GetID();

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    // Write here the main logic of process: TRestRawSignalRecoverSaturationProcess
    // Read data from input event, write data to output event, and save observables to tree

    RESTDebug << "TRestRawSignalRecoverSaturationProcess::ProcessEvent. Event ID : " << fAnaEvent->GetID()
              << RESTendl;
    // observables of the process
    Double_t addedAmplitude = 0;
    Double_t addedIntegral = 0;
    Int_t nSignalsSaturated = 0;
    Int_t nSignalsRecovered = 0;

    // Set the baseline range if it has been provided
    if (fBaseLineRange.X() != -1 && fBaseLineRange.Y() != -1)
        fAnaEvent->SetBaseLineRange(fBaseLineRange.X(),
                                    fBaseLineRange.Y());  // this will also calculate the baseline

    // process each signal in the event
    for (int s = 0; s < fAnaEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* signal = fAnaEvent->GetSignal(s);
        Double_t signalAddedAmplitude = 0;

        // skip signals that are not saturated (if fProcessAllSignals is false)
        if (!signal->IsADCSaturation(fMinSaturatedBins) && !fProcessAllSignals) {
            continue;
        }

        RESTDebug << "Processing signal " << s << " in event " << eventID << RESTendl;
        nSignalsSaturated++;
        Int_t maxPeakBin = signal->GetMaxPeakBin();
        Short_t maxValue = (*signal)[maxPeakBin];
        std::vector<size_t> saturatedBins;

        if (maxValue < fMinSaturationValue) {
            RESTDebug << "    Saturation value " << maxValue << " is less than the minimum value "
                      << fMinSaturationValue << RESTendl;
            continue;
        }

        // Find all saturated bins
        for (size_t i = (size_t)maxPeakBin; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if ((*signal)[i] == maxValue)
                saturatedBins.push_back(i);
            else
                break;  // Stop when the signal stops being saturated
        }

        // If processAllSignals is true, set saturatedBins for all signals
        if (fProcessAllSignals && saturatedBins.size() < fMinSaturatedBins) {
            saturatedBins.clear();
            // set saturated bins around maxPeakBin
            for (size_t i = maxPeakBin - fNBinsIfNotSaturated / 2;
                 i < maxPeakBin + fNBinsIfNotSaturated / 2 && i < (size_t)signal->GetNumberOfPoints(); i++) {
                saturatedBins.push_back(i);
            }
        }

        if (!saturatedBins.empty()) {
            RESTDebug << "    Saturated bins:" << saturatedBins.front() << " to " << saturatedBins.back()
                      << " at " << maxValue << RESTendl;
        }

        // Create TGraph with the not saturated bins for the fit
        TGraph* g = new TGraph();
        for (size_t i = 0; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if (std::find(saturatedBins.begin(), saturatedBins.end(), i) != saturatedBins.end()) continue;
            g->AddPoint(i, (*signal)[i]);
        }
        /* g = signal->GetGraph(); // this would return (*signal)[i]-baseLine (if baseline has been
        // initialized). Then one should remove the saturated bins from the TGraph:
        // Remove the saturated bins from the TGraph representing the signal
        for (size_t i = 0; i < saturatedBins.size(); i++) {
            g->RemovePoint(maxPeakBin);  // when one point is removed the other points are shifted
        } //*/

        // ShaperSin function (AGET theoretic curve) times logistic function. Better without the sin
        Double_t startFitRange = 0;
        Double_t endFitRange = signal->GetNumberOfPoints();
        if (fFitRange.X() != -1 && fFitRange.Y() != -1) {
            startFitRange = fFitRange.X();
            endFitRange = fFitRange.Y();
        }
        TF1* f = new TF1("fit",
                         "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                         "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] / "
                         "(1+TMath::Exp(-10000*(x-[3])))",
                         startFitRange, endFitRange);
        RESTDebug << "    nPoints" << signal->GetNumberOfPoints() << RESTendl;
        RESTDebug << "    Function created" << RESTendl;
        // First estimation of the parameters
        auto peakposEstimate = maxPeakBin + saturatedBins.size() / 2;
        Double_t amplEstimate = maxValue;
        Double_t widthEstimate = (endFitRange - startFitRange) * 0.2;
        Double_t baselineEstimate = 250;

        // Second (and better) estimation of the parameters
        auto pOverThreshold = signal->GetPointsOverThreshold();
        if (!pOverThreshold.empty()) {
            // extrapolate the line connecting the first point of the pulse peak:
            // (pOverThreshold[0], signal->GetRawData(pOverThreshold[0]))
            // with the first point to be saturated: (maxPeakBin, maxValue)
            // to the peak position (peakposEstimate)
            amplEstimate = 0.9 * (maxValue - signal->GetRawData(pOverThreshold[0])) /
                           (maxPeakBin - pOverThreshold[0]) * (peakposEstimate - pOverThreshold[0]);
            // the amplitude estimate should be at least the maximum value of the signal
            if (amplEstimate < maxValue) amplEstimate = maxValue;
        }
        // signal->CalculateBaseLine(20,150);
        if (signal->isBaseLineInitialized()) {
            baselineEstimate = signal->GetBaseLine();
        }
        RESTDebug << "    Estimations: ampl=" << amplEstimate << " width=" << widthEstimate
                  << " baseline=" << baselineEstimate << " peakpos=" << peakposEstimate << RESTendl;
        // Configure the fit parameters
        f->SetParNames("Baseline", "Amplitude", "ShapingTime", "PeakPosition");
        // f->SetParameters(baselineEstimate, amplEstimate / 0.0498, widthEstimate, peakposEstimate -
        // widthEstimate);
        //  Baseline
        f->SetParameter(0, baselineEstimate);
        f->SetParLimits(0, 0, maxValue);  // baseline should be positive and less than the saturation value
        if (signal->isBaseLineInitialized()) {  // fix the baseline to make it faster and more reliable
            f->FixParameter(0, baselineEstimate);
        }  //*/
        // Amplitude
        f->SetParameter(1, amplEstimate / 0.0498);  // 0.0498=e^{-3}
        f->SetParLimits(1, 0,
                        maxValue / 0.0498 * 100);  // max allowed amplitude is 100 times the saturation value
        // Width or shaping time
        f->SetParameter(2, widthEstimate);
        f->SetParLimits(2, 0,
                        signal->GetNumberOfPoints());  // width should be positive and less than the window
        // Peak position
        f->SetParameter(3, peakposEstimate - widthEstimate);
        f->SetParLimits(
            3, 0, signal->GetNumberOfPoints());  // peak position should be positive and less than the window

        std::string fitOptions = "R";
        if (GetVerboseLevel() < TRestStringOutput::REST_Verbose_Level::REST_Debug) {
            fitOptions += "NQ";
        }
        g->Fit(f, fitOptions.c_str());

        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
            g->Draw("AL");
        }
        // Add the recovered signal to the original signal
        TRestRawSignal toAddSignal(0);
        bool anyBinRecovered = false;
        for (size_t i = 0; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if (std::find(saturatedBins.begin(), saturatedBins.end(), i) != saturatedBins.end()) {
                Double_t value = f->Eval(i) - maxValue;
                if (value > 0 || fProcessAllSignals) {
                    toAddSignal.AddPoint(value);
                    anyBinRecovered = true;
                    addedIntegral += value;
                    if (value > signalAddedAmplitude) signalAddedAmplitude = value;
                    RESTExtreme << "    Adding value " << value << RESTendl;
                    continue;
                }
            }
            toAddSignal.AddPoint((Short_t)0);
        }

        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
            f->Draw("same");
            fC->Update();
            std::cin.get();
        }

        delete g;
        delete f;

        if (!anyBinRecovered) {
            RESTDebug << "    Signal " << s << " in event " << eventID << " not recovered" << RESTendl;
            continue;  // nothing to add
        }
        nSignalsRecovered++;
        addedAmplitude += signalAddedAmplitude;
        signal->SignalAddition(toAddSignal);
        RESTDebug << "    Signal " << s << " in event " << eventID << " recovered" << RESTendl;
    }

    SetObservableValue("AddedAmplitude", addedAmplitude);
    SetObservableValue("AddedIntegral", addedIntegral);
    SetObservableValue("SignalsSaturated", nSignalsSaturated);
    SetObservableValue("SignalsRecovered", nSignalsRecovered);

    return fAnaEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestRawSignalRecoverSaturationProcess::EndProcess() {
    // Write here the jobs to do when all the events are processed
}
