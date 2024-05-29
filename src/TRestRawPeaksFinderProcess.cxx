
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

    auto event = fInputEvent->GetSignalEventForTypes(fChannelTypes, fReadoutMetadata);

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelTypes.empty()) {
        cerr << "TRestRawPeaksFinderProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    vector<tuple<UShort_t, UShort_t, double>> eventPeaks;

    for (int signalIndex = 0; signalIndex < event.GetNumberOfSignals(); signalIndex++) {
        const auto signal = event.GetSignal(signalIndex);
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
        } 
        else if (channelType == "veto") {
            // For veto signals the baseline is calculated over the whole range, as we don´t know where the signal will be.
            signal->CalculateBaseLine(0, 511);
            // For veto signals the threshold is selected by the user.
            const auto peaks =
                signal->GetPeaksVeto(signal->GetBaseLine() + fThresholdOverBaseline, fDistance);

            for (const auto& [time, amplitude] : peaks) {
                eventPeaks.emplace_back(signalId, time, amplitude);
            }
        }

        if (fRemoveAllVetos) {
            if (channelType == "veto") {
                fInputEvent->RemoveSignalWithId(signalId);
            }
        }
    }

    // sort eventPeaks by time, then signal id
    sort(eventPeaks.begin(), eventPeaks.end(),
         [](const tuple<UShort_t, UShort_t, double>& a, const tuple<UShort_t, UShort_t, double>& b) {
             return tie(get<1>(a), get<0>(a)) < tie(get<1>(b), get<0>(b));
         });

    if (fRemovePeaklessVetos) {
        // Create a set to store the signal IDs from eventPeaks
        set<UShort_t> peakSignalIds;
        for (const auto& [channelId, time, amplitude] : eventPeaks) {
            peakSignalIds.insert(channelId);
        }

        // Iterate over the signals in the event
        for (int signalIndex = 0; signalIndex < event.GetNumberOfSignals(); signalIndex++) {
            const auto signal = event.GetSignal(signalIndex);
            const UShort_t signalId = signal->GetSignalID();

            // If the signal ID is not in peakSignalIds, remove the signal
            if (peakSignalIds.find(signalId) == peakSignalIds.end()) {
                fInputEvent->RemoveSignalWithId(signalId);
            }
        }
    }

    // SetObservableValue("peaks", eventPeaks); // problems with dictionaries
    vector<UShort_t> peaksChannelId;
    vector<UShort_t> peaksTime;
    vector<double> peaksAmplitude;

    for (const auto& [channelId, time, amplitude] : eventPeaks) {
        peaksChannelId.push_back(channelId);
        peaksTime.push_back(time);
        peaksAmplitude.push_back(amplitude);
    }

    SetObservableValue("peaksChannelId", peaksChannelId);
    SetObservableValue("peaksTime", peaksTime);
    SetObservableValue("peaksAmplitude", peaksAmplitude);

    vector<UShort_t> windowIndex(eventPeaks.size(), 0);  // Initialize with zeros
    vector<UShort_t> windowCenter;  // for each different window, the center of the window

    for (size_t peakIndex = 0; peakIndex < eventPeaks.size(); peakIndex++) {
        const auto& [channelId, time, amplitude] = eventPeaks[peakIndex];
        const auto windowTime = time - fWindow / 2;
        const auto windowEnd = time + fWindow / 2;

        // check if the peak is already in a window
        if (windowIndex[peakIndex] != 0) {
            continue;
        }

        // create a new window
        windowCenter.push_back(time);

        // add the peak to the window
        windowIndex[peakIndex] = windowCenter.size();

        // add the peaks that are in the window
        for (size_t otherPeakIndex = peakIndex + 1; otherPeakIndex < eventPeaks.size(); otherPeakIndex++) {
            const auto& [otherChannelId, otherTime, otherAmplitude] = eventPeaks[otherPeakIndex];

            if (otherTime < windowTime) {
                continue;
            }

            if (otherTime > windowEnd) {
                break;
            }

            windowIndex[otherPeakIndex] = windowCenter.size();
        }
    }

    // subtract 1 from windowIndex so it starts on 0
    for (auto& index : windowIndex) {
        index--;
    }

    // validation
    // check only values from 0 ... windowCenter.size() -1 are in windowIndex
    // ALL values in this range should appear at least once
    for (size_t index = 0; index < windowCenter.size(); index++) {
        bool found = false;
        for (const auto& window : windowIndex) {
            if (window == index) {
                found = true;
                break;
            }
        }
        if (!found) {
            cerr << "TRestRawPeaksFinderProcess::ProcessEvent: window index " << index << " not found"
                 << endl;
            exit(1);
        }
    }

    SetObservableValue("windowIndex", windowIndex);
    SetObservableValue("windowCenter", windowCenter);

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
        cerr << "TRestRawPeaksFinderProcess::InitProcess: removing veto signals only makes sense when the process is applied to veto signals. Remove \"removePeaklessVetos\" parameter" << endl;
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

    RESTMetadata << "Baseline range for tpc signals: " << fBaselineRange.X() << " - " << fBaselineRange.Y() << RESTendl;
    RESTMetadata << "Threshold over baseline for veto signals: " << fThresholdOverBaseline << RESTendl;

    RESTMetadata << "Distance: " << fDistance << RESTendl;
    RESTMetadata << "Window: " << fWindow << RESTendl;

    EndPrintProcess();
}