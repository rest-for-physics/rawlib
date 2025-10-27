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
/// THIS DOCUMENTATION NEEDS REVISION. TODO If possible a figure with the
/// pulse ilustrating the different observables extracted.
///
/// The following metadata parameters might be defined at the RML to control
/// the rawsignal analysis is performed:
/// * **baseLineRange:** The bins from the rawdata samples that will be used
/// to calculate the baseline average and fluctuation.
///
/// * **baseLineOption:** An optional parameter. When set to "ROBUST", the
/// baseline parameters will be calculated using TRestRawSignal's robust
/// methods, instead of mean and standard deviation, the median and IQR sigma
/// are used.
///
/// * **integralRange**: The calculated observables will only consider points
/// found inside this range.
///
/// There are 3 additional parameters that are used in combination to identify
/// the points over threshold at each signal. Those parameters are used, for
/// example, to calculate the number of good signals observable.
///
/// * **pointThreshold**: The number of sigmas over baseline fluctuations to
/// identify a point overthreshold
/// * **signalThreshold**: A parameter to define a minimum signal fluctuation.
/// Measured in sigmas.
/// * **pointsOverThreshold**: The minimum number of points over threshold to
/// identify a signal as such
///
/// Additionaly, there is a metadata parameter,*signalsRange* that allows to
/// define the signal ids over which this process will have effect. This
/// parameter may allow to define different TRestRawSignalAnalysisProcess
/// instances at the RML operating over different signal ranges. For example:
///
/// \code
/// <TRestRawSignalAnalysisProcess name="rawMM" signalsRange="(0,512)"
///        baseLineRange="(5,100)" observables="all" />
///
/// <TRestRawSignalAnalysisProcess name="rawVETO" signalsRange="(513,1024)" >
///		  <parameter name="pointThreshold" value="5" />
///		  <parameter name="signalThreshold" value="3.5" />
///		  <parameter name="pointsOverThreshold" value="15" />
///
///       <observable name="rawAna_max_amplitude_map" type="map<int,double>" />
/// </TRestRawSignalAnalysisProcess>
/// \endcode
///
///
/// ### Observables
///
/// Number of signals and base line:
///
/// * **NumberOfSignals**: Number of pulses recorded in an event.
/// * **NumberOfGoodSignals**: Number of pulses recorded in an event that gets
/// over the threshold. It counts the signal if GetIntegralWithThreshold
/// (fSignal) > 0.
/// * **BaseLineMean**: Average of the base line of the pulses of the event.
/// * **BaseLineSigmaMean**: Average of the standard deviation from the base
/// line in
/// the pulses of the event.
///
/// Integrals and energy estimations:
///
/// * **FullIntegral**: Add the integral of all pulses of the event. The
/// integral of
/// a pulse is the sum of all samples.
/// * **TripleMaxIntegral**: Add the highest value of each pulse with the
/// previous
/// sample and the next, and then add this amount for all pulses in the event.
/// It
/// is an estimation of the deposited energy.
/// * **ThresholdIntegral**: Add the integral of all the pulses in the event
/// that pass
/// the threshold.
/// * **SlopeIntegral**: Add the integral of the rising part of each pulse of
/// the event.
/// * **RiseSlopeAvg**: Add the SlopeIntegral of all pulses in the event that
/// pass the
/// GetThresholdIntegralValue() > 0 condition and divides it by the number of
/// signals
/// that pass the cut.
/// * **IntegralBalance**: (fullIntegral - thrIntegral) / (fullIntegral +
/// thrIntegral)
/// Balance between FullIntegral and ThresholdIntegral.
/// * **RateOfChangeAvg**: RiseSlopeAvg/SlopeIntegral. The first value takes
/// only the
/// events that pass the threshold cut, SopeIntegral takes all the events.
/// * **xEnergySum**: Add the ThresholdIntegral of all signals recorded in X
/// direction.
/// * **yEnergySum**: Add the ThresholdIntegral of all signals recorded in Y
/// direction.
///
///
/// Time observables:
///
/// * **TimeBinsLength**: MaxTime of the event - MinTime of the event. The functions
/// GetMaxTime() and GetMinTime() take the number of points of the signal
/// (MinTime=0 MaxTime=fSignal[0].GetNumberOfPoints()).
/// * **RiseTimeAvg**: Add GetRiseTime(fSignal) for all signals that satisfy the
/// GetThresholdIntegralValue() > 0 condition and divide it by the number of signals
/// that pass this cut. GetRiseTime(fSignal) provides the number of bins between
/// the fist sample that pass the threshold and the maximum of the peak.
///
/// Peak amplitude observables:
///
/// * **AmplitudeIntegralRatio**: ThresholdIntegral/maxValueIntegral. This is
/// the sum
/// of the integral of all the pulses in the event that pass the threshold
/// divided by
/// the sum of the maximum point of these pulses.
/// * **MinPeakAmplitude**: Minimum value between the maximum points of the
/// pulses that
/// pass the threshold in the event.
/// * **MaxPeakAmplitude**: Maximum value between the maximum points of the
/// pulses that
/// pass the threshold in the event.
/// * **PeakAmplitudeIntegral**: Sum of all maximum points of the pulses that
/// pass the
/// threshold in the event.
/// * **AmplitudeRatio**: PeakAmplitudeIntegral/ MaxPeakAmplitude. Sum of all
/// maximum
/// points divided by the maximum between the maximum points of the pulses.
/// * **MinEventValue**: Minimum value reached by a pulse in the event.
///
/// Peak time observables:
///
/// * **MaxPeakTime**: Highest bin for the maximum point of a pulse that pass
/// the
/// threshold in the event.
/// * **MinPeakTime**: Smallest bin for the maximum point of a pulse that pass
/// the
/// threshold in the event.
/// * **MaxPeakTimeDelay**: MaxPeakTime-MinPeakTime. Time between the earliest
/// peak and
/// the latest peak between the pulses that pass the threshold in the event.
/// * **AveragePeakTime**: For all pulses that pass the threshold, add the bin
/// of their
/// maximums and divide this amount by the number of signals that pass the
/// threshold.
///
/// Observables for individual signal info:
///
/// * **risetime**: Map the ID of each signal in the event with its rise time.
/// GetRiseTime() gives the number of bins between the first sample that pass
/// the
/// threshold and the maximum point of the peak.
/// * **risetimemean**: Add the rise time of all pulses in the event and divide
/// it by
/// the number of pulses.
/// * **baseline**: Map the ID of each signal in the event with its base line.
/// It
/// computes the base line adding the samples in a certain range and dividing it
/// by
/// the number of samples.
/// * **baselinemean**: Add the base line of all pulses in the event and divide
/// it by
/// the number of pulses.
/// * **baselinesigma**: Map the ID of each signal in the event with its base
/// line's
/// standard deviation. Standard deviation computed as the squared root of the
/// sum of
/// the squared differences between the samples and the base line, divided by
/// the number
/// of samples in the range used to compute the base line.
/// * **baselinesigmamean**: Add the base line's standard deviation of all
/// pulses in
/// the event and divide it by the number of pulses.
/// * **max_amplitude_map**: Map the ID of each signal in the event with the
/// amplitude
/// of its peak. Amplitude = MaxPeakValue-BaseLine.
/// * **ampeve_maxmethod**: Add the amplitude of all pulses in the event.
/// Amplitude = MaxPeakValue-BaseLine.
/// * **thr_integral_map**: Map the ID of each signal in the event with the
/// threshold
/// integral of its peak. GetIntegralWithThreshold() adds the samples of a pulse
/// that get over the threshold. A certain number of samples must pass the
/// threshold
/// to be taken into account.
/// * **ampeve_intmethod**: Add the threshold integral of all pulses in the
/// event.
/// GetIntegralWithThreshold () adds the samples of a pulse that get over the
/// threshold.
/// A certain number of samples must pass the threshold to be taken into
/// account.
///
///
/// You may add filters to any observable inside the analysis tree. To add a cut,
/// write "cut" sections in your rml file:
///
/// \code
/// <TRestRawSignalAnalysisProcess name=""  ... >
///     <cut name="MeanBaseLineCut" value="(0,4096)" />
/// </TRestRawSignalAnalysisProcess>
/// \endcode
///
/// <hr>
///
/// \warning **⚠ REST is under continuous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find will using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///_______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2017-February: First implementation of raw signal analysis process into
/// REST_v2.
///                Created from TRestDetectorSignalAnalysisProcess
///
/// \class      TRestRawSignalAnalysisProcess
/// \author     Javier Galan
///
/// <hr>
///

#include "TRestRawSignalAnalysisProcess.h"

using namespace std;

ClassImp(TRestRawSignalAnalysisProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalAnalysisProcess::TRestRawSignalAnalysisProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalAnalysisProcess::~TRestRawSignalAnalysisProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalAnalysisProcess::InitProcess() {
    if (fSignalsRange.X() != -1 && fSignalsRange.Y() != -1) {
        fRangeEnabled = true;
    }
}

void TRestRawSignalAnalysisProcess::InitFromConfigFile() {
    TRestEventProcess::InitFromConfigFile();
    const auto filterType = GetParameter("channelType", "");
    if (!filterType.empty()) {
        fChannelTypes.insert(filterType);
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalAnalysisProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fInputEvent->InitializeReferences(run);
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelTypes.empty()) {
        cerr << "TRestRawSignalAnalysisProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    auto event = fInputEvent->GetSignalEventForTypes(fChannelTypes, fReadoutMetadata);

    // we save some complex typed analysis result
    map<int, Double_t> baseline;
    map<int, Double_t> baselinesigma;
    map<int, Double_t> ampsgn_maxmethod;
    map<int, Double_t> ampsgn_intmethod;
    map<int, int> risetime;
    map<int, int> peak_time;
    map<int, int> npointsot;
    vector<int> saturatedchnId;

    baseline.clear();
    baselinesigma.clear();
    ampsgn_maxmethod.clear();
    ampsgn_intmethod.clear();
    risetime.clear();
    npointsot.clear();

    Int_t nGoodSignals = 0;

    /// We define (or re-define) the baseline range and calculation range of our
    /// raw-signals.
    // This will affect the calculation of observables, but not the stored
    // TRestRawSignal data.
    event.SetBaseLineRange(fBaseLineRange, fBaseLineOption);
    event.SetRange(fIntegralRange);

    for (int s = 0; s < event.GetNumberOfSignals(); s++) {
        TRestRawSignal* sgnl = event.GetSignal(s);

        /// Important call we need to initialize the points over threshold in a TRestRawSignal
        sgnl->InitializePointsOverThreshold(TVector2(fPointThreshold, fSignalThreshold),
                                            fPointsOverThreshold);

        if (fRangeEnabled && (sgnl->GetID() < fSignalsRange.X() || sgnl->GetID() > fSignalsRange.Y())) {
            continue;
        }

        // We do not want that signals that are not identified as such contribute to
        // define our observables
        // nkx: we still need to store all the signals in baseline/rise time maps in
        // case for noise analysis
        // if (sgnl->GetPointsOverThreshold().size() < 2) continue;
        if (sgnl->GetPointsOverThreshold().size() >= 2) nGoodSignals++;

        // Now TRestRawSignal returns directly baseline subtracted values
        baseline[sgnl->GetID()] = sgnl->GetBaseLine();
        baselinesigma[sgnl->GetID()] = sgnl->GetBaseLineSigma();
        ampsgn_intmethod[sgnl->GetID()] = sgnl->GetThresholdIntegral();
        ampsgn_maxmethod[sgnl->GetID()] = sgnl->GetMaxPeakValue();
        risetime[sgnl->GetID()] = sgnl->GetRiseTime();
        peak_time[sgnl->GetID()] = sgnl->GetMaxPeakBin();
        npointsot[sgnl->GetID()] = sgnl->GetPointsOverThreshold().size();
        if (sgnl->IsADCSaturation()) saturatedchnId.push_back(sgnl->GetID());
    }

    SetObservableValue("pointsoverthres_map", npointsot);
    SetObservableValue("risetime_map", risetime);
    SetObservableValue("peak_time_map", peak_time);
    SetObservableValue("baseline_map", baseline);
    SetObservableValue("baselinesigma_map", baselinesigma);
    SetObservableValue("max_amplitude_map", ampsgn_maxmethod);
    SetObservableValue("thr_integral_map", ampsgn_intmethod);
    SetObservableValue("SaturatedChannelID", saturatedchnId);

    Double_t baseLineMean = event.GetBaseLineAverage();
    SetObservableValue("BaseLineMean", baseLineMean);

    Double_t baseLineSigma = event.GetBaseLineSigmaAverage();
    SetObservableValue("BaseLineSigmaMean", baseLineSigma);

    Double_t timeDelay = event.GetMaxTime() - event.GetMinTime();
    SetObservableValue("TimeBinsLength", timeDelay);

    Int_t nSignals = event.GetNumberOfSignals();
    SetObservableValue("NumberOfSignals", nSignals);
    SetObservableValue("NumberOfGoodSignals", nGoodSignals);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // SubstractBaselines
    // After this the signal gets zero-ed, for the following analysis
    // Keep in mind, to add raw signal analysis, we must write code at before
    // This is where most of the problems occur
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Javier: I believe we should not substract baseline in the analysis process
    // then ...
    // ... of course we need to consider baseline substraction for each
    // observable. TRestRawSignal methods
    // should do that internally. I have updated that to be like that, but we need
    // to be with open eyes for
    // some period.
    // Baseline substraction will always happen when we transfer a TRestRawSignal
    // to TRestDetectorSignal
    //
    // We do not substract baselines then now, as it was done before
    //
    // fInputEvent->SubstractBaselines(fBaseLineRange.X(), fBaseLineRange.Y());
    //
    // Methods in TRestRawSignal have been updated to consider baseline.
    // TRestRawSignal now implements that internally. We need to define the
    // baseline range, and the range
    // where calculations take place. All we need is to call at some point to the
    // following methods.
    //
    // TRestRawSignalEvent::SetBaseLineRange and TRestRawSignalEvent::SetRange.
    //
    // Then, if any method accepts a different range it will be given in the
    // method name,
    // for example: GetIntegralInRange( Int_t startBin, Int_t endBin );
    //

    Double_t fullIntegral = event.GetIntegral();
    SetObservableValue("FullIntegral", fullIntegral);

    Double_t thrIntegral = event.GetThresholdIntegral();
    SetObservableValue("ThresholdIntegral", thrIntegral);

    Double_t riseSlope = event.GetRiseSlope();
    SetObservableValue("RiseSlopeAvg", riseSlope);

    Double_t slopeIntegral = event.GetSlopeIntegral();
    SetObservableValue("SlopeIntegral", slopeIntegral);

    Double_t rateOfChange = riseSlope / slopeIntegral;
    if (slopeIntegral == 0) rateOfChange = 0;
    SetObservableValue("RateOfChangeAvg", rateOfChange);

    Double_t riseTime = event.GetRiseTime();
    SetObservableValue("RiseTimeAvg", riseTime);

    Double_t tripleMaxIntegral = event.GetTripleMaxIntegral();
    SetObservableValue("TripleMaxIntegral", tripleMaxIntegral);

    Double_t integralRatio = (fullIntegral - thrIntegral) / (fullIntegral + thrIntegral);
    SetObservableValue("IntegralBalance", integralRatio);

    Double_t maxValue = 0;
    Double_t minValue = 1.e6;
    Double_t maxValueIntegral = 0;
    Double_t minDownValue = 1.e6;

    Double_t minPeakTime = 1000;  // TODO substitute this for something better
    Double_t maxPeakTime = 0;
    Double_t peakTimeAverage = 0;

    for (int s = 0; s < event.GetNumberOfSignals(); s++) {
        TRestRawSignal* sgnl = event.GetSignal(s);

        if (fRangeEnabled && (sgnl->GetID() < fSignalsRange.X() || sgnl->GetID() > fSignalsRange.Y()))
            continue;

        if (sgnl->GetPointsOverThreshold().size() > 1) {
            Double_t value = event.GetSignal(s)->GetMaxValue();
            maxValueIntegral += value;

            if (value > maxValue) maxValue = value;
            if (value < minValue) minValue = value;

            Double_t peakBin = sgnl->GetMaxPeakBin();
            peakTimeAverage += peakBin;

            if (minPeakTime > peakBin) minPeakTime = peakBin;
            if (maxPeakTime < peakBin) maxPeakTime = peakBin;
        }
        Double_t mindownvalue = event.GetSignal(s)->GetMinValue();
        if (mindownvalue < minDownValue) {
            minDownValue = mindownvalue;
        }
    }

    if (nGoodSignals > 0) peakTimeAverage /= nGoodSignals;

    Double_t ampIntRatio = thrIntegral / maxValueIntegral;
    if (maxValueIntegral == 0) ampIntRatio = 0;

    SetObservableValue("AmplitudeIntegralRatio", ampIntRatio);
    SetObservableValue("MinPeakAmplitude", minValue);
    SetObservableValue("MaxPeakAmplitude", maxValue);
    SetObservableValue("PeakAmplitudeIntegral", maxValueIntegral);

    SetObservableValue("MinEventValue", minDownValue);

    Double_t amplitudeRatio = maxValueIntegral / maxValue;
    if (maxValue == 0) amplitudeRatio = 0;

    SetObservableValue("AmplitudeRatio", amplitudeRatio);
    SetObservableValue("MaxPeakTime", maxPeakTime);
    SetObservableValue("MinPeakTime", minPeakTime);

    Double_t peakTimeDelay = maxPeakTime - minPeakTime;

    SetObservableValue("MaxPeakTimeDelay", peakTimeDelay);
    SetObservableValue("AveragePeakTime", peakTimeAverage);

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
        for (const auto& i : fObservablesDefined) {
            fAnalysisTree->PrintObservable(i.second);
        }
    }

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) {
        return nullptr;
    }

    return fInputEvent;
}
