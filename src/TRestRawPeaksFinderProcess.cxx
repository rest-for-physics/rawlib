
#include "TRestRawPeaksFinderProcess.h"

#include <utility>

ClassImp(TRestRawPeaksFinderProcess);

using namespace std;

void TRestRawPeaksFinderProcess::InitProcess() {}

TRestEvent* TRestRawPeaksFinderProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fInputEvent->InitializeReferences(run);
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelTypes.empty()) {
        cerr << "TRestRawPeaksFinderProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    vector<tuple<UShort_t, UShort_t, double>> eventPeaks;  // signalId, time, amplitude

    for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
        const auto signal = fInputEvent->GetSignal(signalIndex);
        const UShort_t signalId = signal->GetSignalID();

        const string channelType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);
        const string channelName = fReadoutMetadata->GetNameForChannelDaqId(signalId);

        // check if channel type is in the list of selected channel types
        if (fChannelTypes.find(channelType) == fChannelTypes.end()) {
            continue;
        }

        // Choose appropriate function based on channel type
        if (channelType == "tpc") {
            signal->CalculateBaseLine(fBaselineRange.X(), fBaselineRange.Y());
            const auto peaks =
                signal->GetPeaks(signal->GetBaseLine() + 5 * signal->GetBaseLineSigma(), fDistance);

            for (const auto& [time, amplitude] : peaks) {
                eventPeaks.emplace_back(signalId, time, amplitude);
            }
        } else if (channelType == "veto") {
            // For veto signals the baseline is calculated over the whole range, as we don´t know where the
            // signal will be.
            signal->CalculateBaseLine(0, 511, "robust");
            // For veto signals the threshold is selected by the user.
            const auto peaks =
                signal->GetPeaksVeto(signal->GetBaseLine() + fThresholdOverBaseline, fDistance);

            for (const auto& [time, amplitude] : peaks) {
                eventPeaks.emplace_back(signalId, time, amplitude);
            }
        }
    }

    // sort eventPeaks by time, then signal id
    sort(eventPeaks.begin(), eventPeaks.end(),
         [](const tuple<UShort_t, UShort_t, double>& a, const tuple<UShort_t, UShort_t, double>& b) {
             return tie(get<1>(a), get<0>(a)) < tie(get<1>(b), get<0>(b));
         });

    vector<UShort_t> peaksChannelId;
    vector<UShort_t> peaksTime;
    vector<double> peaksAmplitude;

    double peaksEnergy = 0.0;
    UShort_t peaksCount = 0;
    UShort_t peaksCountUnique = 0;

    set<UShort_t> uniquePeaks;
    for (const auto& [channelId, time, amplitude] : eventPeaks) {
        peaksChannelId.push_back(channelId);
        peaksTime.push_back(time);
        peaksAmplitude.push_back(amplitude);

        peaksEnergy += amplitude;
        peaksCount++;

        if (uniquePeaks.find(channelId) == uniquePeaks.end()) {
            uniquePeaks.insert(channelId);
            peaksCountUnique++;
        }
    }

    SetObservableValue("peaksChannelId", peaksChannelId);
    SetObservableValue("peaksTime", peaksTime);
    SetObservableValue("peaksAmplitude", peaksAmplitude);

    SetObservableValue("totalPeaksEnergy", peaksEnergy);
    SetObservableValue("peaksCount", peaksCount);
    SetObservableValue("peaksCountUnique", peaksCountUnique);

    vector<UShort_t> windowPeakIndex(eventPeaks.size(), 0);
    vector<UShort_t> windowPeakCenter(eventPeaks.size(), 0);
    vector<UShort_t> windowPeakMultiplicity(eventPeaks.size(), 0);

    UShort_t window_index = 0;
    for (size_t peakIndex = 0; peakIndex < eventPeaks.size(); peakIndex++) {
        const auto& [channelId, time, amplitude] = eventPeaks[peakIndex];
        const auto windowTimeStart = time - fWindow / 2;
        const auto windowTimeEnd = time + fWindow / 2;

        // check if the peak is already in a window
        if (windowPeakIndex[peakIndex] != 0) {
            continue;
        }

        UShort_t window_center_time = time;

        set<UShort_t> windowPeaksIndex;

        // add the peak to the window
        windowPeaksIndex.insert(peakIndex);

        // add the peaks that are in the window
        for (size_t otherPeakIndex = peakIndex + 1; otherPeakIndex < eventPeaks.size(); otherPeakIndex++) {
            const auto& [otherChannelId, otherTime, otherAmplitude] = eventPeaks[otherPeakIndex];

            if (otherTime < windowTimeStart) {
                continue;
            }

            if (otherTime > windowTimeEnd) {
                break;
            }

            windowPeaksIndex.insert(otherPeakIndex);
        }

        for (const auto& index : windowPeaksIndex) {
            windowPeakIndex[index] = window_index;
            windowPeakCenter[index] = window_center_time;
            windowPeakMultiplicity[index] = windowPeaksIndex.size();
        }

        window_index++;
    }

    SetObservableValue("windowPeakIndex", windowPeakIndex);
    SetObservableValue("windowPeakCenter", windowPeakCenter);
    SetObservableValue("windowPeakMultiplicity", windowPeakMultiplicity);

    vector<UShort_t> windowCenter;
    vector<UShort_t> windowMultiplicity;

    set<UShort_t> windowPeaksIndexInserted;
    for (size_t peakIndex = 0; peakIndex < eventPeaks.size(); peakIndex++) {
        const auto& window_peak_index = windowPeakIndex[peakIndex];
        if (windowPeaksIndexInserted.find(window_peak_index) != windowPeaksIndexInserted.end()) {
            continue;
        }

        windowPeaksIndexInserted.insert(window_peak_index);

        windowCenter.push_back(windowPeakCenter[peakIndex]);
        windowMultiplicity.push_back(windowPeakMultiplicity[peakIndex]);
    }

    SetObservableValue("windowCenter", windowCenter);
    SetObservableValue("windowMultiplicity", windowMultiplicity);

    // Remove peak-less veto signals after the peak finding if chosen
    if (fRemovePeaklessVetos && !fRemoveAllVetos) {
        set<UShort_t> peakSignalIds;
        for (const auto& [channelId, time, amplitude] : eventPeaks) {
            peakSignalIds.insert(channelId);
        }

        vector<UShort_t> signalsToRemove;
        for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
            const auto signal = fInputEvent->GetSignal(signalIndex);
            const UShort_t signalId = signal->GetSignalID();
            const string signalType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);

            if (signalType == "veto" && peakSignalIds.find(signalId) == peakSignalIds.end()) {
                signalsToRemove.push_back(signalId);
            }
        }

        // Now remove all veto signals identified
        for (const auto& signalId : signalsToRemove) {
            fInputEvent->RemoveSignalWithId(signalId);
        }
    }

    // Remove all veto signals after the peak finding if chosen
    if (fRemoveAllVetos) {
        vector<UShort_t> signalsToRemove;
        for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
            const auto signal = fInputEvent->GetSignal(signalIndex);
            const UShort_t signalId = signal->GetSignalID();
            const string signalType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);

            if (signalType == "veto") {
                signalsToRemove.push_back(signalId);
            }
        }

        // Now remove all veto signals identified
        for (const auto& signalId : signalsToRemove) {
            fInputEvent->RemoveSignalWithId(signalId);
        }
    }

    return inputEvent;
}

void TRestRawPeaksFinderProcess::InitFromConfigFile() {
    const auto filterType = GetParameter("channelType", "");
    if (!filterType.empty()) {
        fChannelTypes.insert(filterType);
    }

    if (fChannelTypes.empty()) {
        // if no channel type is specified, use all channel types
    }

    fThresholdOverBaseline = StringToDouble(GetParameter("thresholdOverBaseline", fThresholdOverBaseline));
    fBaselineRange = Get2DVectorParameterWithUnits("baselineRange", fBaselineRange);
    fDistance = StringToDouble(GetParameter("distance", fDistance));
    fWindow = StringToDouble(GetParameter("window", fWindow));
    fRemoveAllVetos = StringToBool(GetParameter("removeAllVetos", fRemoveAllVetos));
    fRemovePeaklessVetos = StringToBool(GetParameter("removePeaklessVetos", fRemovePeaklessVetos));

    if (fBaselineRange.X() > fBaselineRange.Y() || fBaselineRange.X() < 0 || fBaselineRange.Y() < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: baseline range is not sorted or < 0" << endl;
        exit(1);
    }

    if (fThresholdOverBaseline < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: threshold over baseline is < 0" << endl;
        exit(1);
    }

    if (fDistance <= 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: distance is < 0" << endl;
        exit(1);
    }

    if (fWindow <= 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: window is < 0" << endl;
        exit(1);
    }

    if (filterType != "veto" && fRemovePeaklessVetos) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: removing veto signals only makes sense when the "
                "process is applied to veto signals. Remove \"removePeaklessVetos\" parameter"
             << endl;
        exit(1);
    }
}

void TRestRawPeaksFinderProcess::PrintMetadata() {
    BeginPrintProcess();

    RESTMetadata << "Applying process to channel types: ";
    for (const auto& type : fChannelTypes) {
        RESTMetadata << type << " ";
    }
    RESTMetadata << RESTendl;

    RESTMetadata << "Baseline range for tpc signals: " << fBaselineRange.X() << " - " << fBaselineRange.Y()
                 << RESTendl;
    RESTMetadata << "Threshold over baseline for veto signals: " << fThresholdOverBaseline << RESTendl;

    RESTMetadata << "Distance: " << fDistance << RESTendl;
    RESTMetadata << "Window: " << fWindow << RESTendl;

    EndPrintProcess();
}
