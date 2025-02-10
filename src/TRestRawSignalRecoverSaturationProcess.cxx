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
/// This class processes signals that have saturated the ADC, recovering
/// lost information using a fit. If the fit is successful, the saturated
/// points are replaced by the corresponding fit values.
/// The process reconstructs the lost signal data due to ADC saturation,
/// allowing further event analysis as if no saturation had occurred.
/// For example, the following image compares the original to the recovered
/// event and the ThresholdIntegral spectrum in TRestRawSignalAnalysisProcess
/// before (red) and after (blue) applying TRestRawSignalRecoverSaturationProcess:
///
/// \image html RecoverSignalProcess_eventRecovered.png "Recovered Event" width=900px
/// \image html RecoverSignalProcess_spectrumComparison.png "Spectrum Comparison" width=500px
///
///
/// ### Fitting Function
/// The fitting function is fixed (hardcoded) to the AGET response function
/// (without the sine term) multiplied by a logistic function:
///
/// \code
/// [0] + [1] * TMath::Exp(-3. * (x-[3])/[2]) *
/// (x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] /
/// (1 + TMath::Exp(-10000 * (x-[3])))
///
/// [0] = "Baseline"
/// [1] = "Amplitude * e^{3}"
/// [2] = "HalfWidth"
/// [3] = "PulseStart"
/// \endcode
///
///
/// \image html RecoverSignalProcess_signalFit.png "Signal Fit" width=500px
///
/// ### Parameters
/// Basic parameters of the process meant to configure the identification of saturated signals to process:
/// - **minSaturatedBins**: Minimum number of saturated bins required to classify a signal as saturated (default: 3).
/// - **minSaturationValue**: Threshold value for considering a signal as saturated (default: 0).
/// - **processAllSignals**: If `false` (default), only saturated signals are processed.
///   If `true`, all signals are processed.
/// - **nBinsIfNotSaturated**: Number of bins treated as 'saturated' when processing all signals.
///   Has no effect when `processAllSignals` is `false`. Default: 20.
///
/// Advanced parameters to configure the fitting process:
/// - **fitRange**: Range of bins used for fitting. If not provided, the entire signal is used.
/// Default: (-1, -1), meaning the whole signal is used.  
/// - **baseLineRange**: Range of bins used to calculate the baseline. If provided, the baseline
///   will be fixed in the fit, improving speed and reliability.
/// Default: (-1, -1), meaning no baseline calculation.  
/// - **initPointsOverThreshold**: Parameters used as input to `TRestRawSignal::InitializePointsOverThreshold`
/// to improve amplitude and width estimation. These parameters are wrapped into a `TVector3`:
/// (`pointThreshold`, `signalThreshold`, `pointsOverThreshold`).
/// Default: (-1, -1, -1), meaning no initialization. \n
///   - `pointThreshold`: Number of standard deviations above baseline fluctuations required
///     to identify a point as over-threshold.  
///   - `signalThreshold`: Minimum required signal fluctuation, measured in standard deviations.  
///   - `pointsOverThreshold`: Minimum number of points over threshold needed to classify a signal as such.  
///
///
/// ### Observables
/// The process adds the following observables to the event:
/// - **addedAmplitude**: Sum of the extra amplitude (new amplitude - previous amplitude)
///   from recovered saturated signals.
/// - **addedIntegral**: Sum of the extra integral (sum of new value - previous value in saturated bins)
///   from recovered signals.
/// - **saturatedSignals**: Number of signals detected as saturated in the event.
/// - **recoveredSignals**: Number of saturated signals successfully modified by the fitted pulse.
///   Always ≤ `saturatedSignals`. If lower, it means the fit values were below the original
///   signal values, so no replacements were made.
///
/// \image html RecoverSignalProcess_addedAmplIntegral.png "Added Amplitude and Integral" width=500px
///
/// ### Examples
/// Example of usage with a minimal RML configuration:
/// \code
/// <addProcess type="TRestRawSignalRecoverSaturationProcess" name="recSat" value="ON" observable="all">
///     <parameter name="minSaturationValue" value="3500" />
///     <parameter name="baseLineRange" value="(20,150)" />
///     <parameter name="fitRange" value="(150,300)" />
/// </addProcess>
/// \endcode
///
/// Example of a more complex RML configuration:
/// \code
/// <addProcess type="TRestRawSignalRecoverSaturationProcess" name="recSat" value="ON" verboseLevel="info" observable="all">
/// 	<parameter name="minSaturatedBins" value="3" />
/// 	<parameter name="minSaturationValue" value="3500" />
/// 	<parameter name="fitRange" value="(150,300)" />
/// 	<parameter name="baseLineRange" value="(20,150)" />
/// 	<parameter name="initPointsOverThreshold" value="(3.5, 2.5, 7)" />
/// </addProcess>
/// \endcode
///
/// Example of a testing RML configuration:
/// \code
/// <addProcess type="TRestRawSignalRecoverSaturationProcess" name="recSat" value="ON" verboseLevel="extreme" observable="all">
/// 	<parameter name="processAllSignals" value="true" />
/// 	<parameter name="nBinsIfNotSaturated" value="16" />
/// 	<parameter name="fitRange" value="(150,300)" />
/// 	<parameter name="baseLineRange" value="(20,150)" />
/// 	<parameter name="initPointsOverThreshold" value="(3.5, 2.5, 7)" />
/// </addProcess>
/// \endcode
///
///----------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2025-Jan: First implementation of TRestRawSignalRecoverSaturationProcess
/// by Álvaro Ezquerro
///
/// \class TRestRawSignalRecoverSaturationProcess
/// \author Álvaro Ezquerro
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
    fInitPointsOverThreshold = TVector3(-1, -1, -1);  // -1 means no initialization
    fC = nullptr;
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
            // maxPeakBin should be the first saturated bin
            maxPeakBin = saturatedBins.empty() ? maxPeakBin : saturatedBins.front();
            maxValue = (*signal)[maxPeakBin];
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

        // First estimation of the parameters
        auto peakposEstimate = maxPeakBin + saturatedBins.size() / 2; // maxPeakBin is the first saturated bin
        Double_t amplEstimate = maxValue;
        Double_t widthEstimate = (endFitRange - startFitRange) * 0.33; // 0.33 is somehow arbitrary
        Int_t binAtHalfMaximum = (Int_t) startFitRange;
        for (size_t i = (size_t) startFitRange; i < (size_t) endFitRange; i++) {
            if ((*signal)[i] > amplEstimate / 2) {
                binAtHalfMaximum = i;
                break;
            }
        }
        widthEstimate = peakposEstimate - binAtHalfMaximum > 0 ? peakposEstimate - binAtHalfMaximum : widthEstimate;
        Double_t baselineEstimate = (*signal)[0]; // first point of the signal is usually part of the baseline
        if (signal->isBaseLineInitialized()) { // if the baseline has been initialized, use it
            baselineEstimate = signal->GetBaseLine();
        }

        // Second (and better) estimation of amplitude and width. It needs to initialize the points over threshold
        Double_t pointThreshold = fInitPointsOverThreshold.X();
        Double_t signalThreshold = fInitPointsOverThreshold.Y();
        Int_t pointsOverThreshold = fInitPointsOverThreshold.Z();
        if (pointsOverThreshold > 0 && pointThreshold > 0 && signalThreshold > 0) {
            signal->InitializePointsOverThreshold(TVector2(pointThreshold, signalThreshold), pointsOverThreshold,
                                                  signal->GetNumberOfPoints()); // we dont care about overshoot here
        }
        auto pOverThreshold = signal->GetPointsOverThreshold();
        if (!pOverThreshold.empty()) {
            RESTDebug << "    Points over threshold: " << pOverThreshold.size() << ". From point "
                      << pOverThreshold.front() << " to " << pOverThreshold.back() << RESTendl;
            // extrapolate the line connecting the first point of the pulse peak:
            // (pOverThreshold[0], signal->GetRawData(pOverThreshold[0]))
            // with the first point to be saturated: (maxPeakBin, maxValue)
            // to the peak position (peakposEstimate)
            amplEstimate = 0.9 * (maxValue - signal->GetRawData(pOverThreshold[0])) /
                           (maxPeakBin - pOverThreshold[0]) * (peakposEstimate - pOverThreshold[0]);
            // the amplitude estimate should be at least the maximum value of the signal
            if (amplEstimate < maxValue) amplEstimate = maxValue;

            // estimate the width as the distance between the first pulse point and the
            // peak position
            widthEstimate = peakposEstimate - pOverThreshold[0];
        }

        RESTDebug << "    Estimations: ampl=" << amplEstimate << " (" << amplEstimate / 0.0498
                  << ")  width=" << widthEstimate << " baseline=" << baselineEstimate << " peakpos="
                  << peakposEstimate << " (" << peakposEstimate - widthEstimate  << ")" << RESTendl;
        // Configure the fit parameters
        f->SetParNames("Baseline", "Amplitude*e^{3}", "HalfWidth", "PulseStart");
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
