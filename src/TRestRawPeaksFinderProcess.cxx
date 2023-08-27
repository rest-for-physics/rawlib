//
// Created by lobis on 24-Aug-23.
//

#include "TRestRawPeaksFinderProcess.h"

#include <utility>

#include "TRestRawReadoutMetadata.h"

ClassImp(TRestRawPeaksFinderProcess);

using namespace std;

TRestRawReadoutMetadata* TRestRawPeaksFinderProcess::Metadata = nullptr;

void TRestRawPeaksFinderProcess::InitProcess() {
    fReadoutMetadata = TRestRawPeaksFinderProcess::Metadata;

    if (!fReadoutMetadata) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: raw readout metadata is null" << endl;
        exit(1);
    }

    for (const auto& type : fChannelTypes) {
        for (const auto& channelId : fReadoutMetadata->GetChannelIDsForType(type)) {
            fChannelIds.insert(channelId);
        }
    }
}

TRestEvent* TRestRawPeaksFinderProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    std::vector<tuple<UShort_t, UShort_t, double>> eventPeaks;

    for (int signalIndex = 0; signalIndex < fSignalEvent->GetNumberOfSignals(); signalIndex++) {
        const auto signal = fSignalEvent->GetSignal(signalIndex);
        const UShort_t channelId = signal->GetSignalID();

        if (fChannelIds.find(channelId) == fChannelIds.end()) {
            continue;
        }

        const string channelType = fReadoutMetadata->GetChannelType(channelId);
        const string channelName = fReadoutMetadata->GetChannelName(channelId);

        // check if channel type is in the list of selected channel types
        if (fChannelTypes.find(channelType) == fChannelTypes.end()) {
            continue;
        }

        signal->CalculateBaseLine(0, 5);
        const auto peaks = signal->GetPeaks(signal->GetBaseLine() + 1.0, fDistance);

        for (const auto& [time, amplitude] : peaks) {
            eventPeaks.emplace_back(channelId, time, amplitude);
        }
        /*
        cout << "Signal ID: " << channelId << " Name: " << channelName << endl;
        for (const auto& [time, amplitude] : peaks) {
            cout << "   - Peak at " << time << " with amplitude " << amplitude << endl;
        }
        */
    }

    // sort eventPeaks by time, then signal id
    sort(eventPeaks.begin(), eventPeaks.end(),
         [](const tuple<UShort_t, UShort_t, double>& a, const tuple<UShort_t, UShort_t, double>& b) {
             return std::tie(std::get<1>(a), std::get<0>(a)) < std::tie(std::get<1>(b), std::get<0>(b));
         });

    SetObservableValue("peaks", eventPeaks);

    std::vector<UShort_t> windowIndex(eventPeaks.size(), 0);  // Initialize with zeros
    std::vector<UShort_t> windowCenter;  // for each different window, the center of the window

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

    SetObservableValue("windowIndex", windowIndex);
    SetObservableValue("windowCenter", windowCenter);

    return inputEvent;
}

void TRestRawPeaksFinderProcess::InitFromConfigFile() {
    const auto filterType = GetParameter("channelType", "");
    if (!filterType.empty()) {
        fChannelTypes.insert(filterType);
    }

    cout << "TRestRawPeaksFinderProcess::InitProcess: filter type: " << filterType << endl;

    if (fChannelTypes.empty()) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: no channel types selected" << endl;
        exit(1);
    }

    fThresholdOverBaseline = StringToDouble(GetParameter("thresholdOverBaseline", fThresholdOverBaseline));
    fBaselineRange = Get2DVectorParameterWithUnits("baselineRange", fBaselineRange);
    fDistance = StringToDouble(GetParameter("distance", fDistance));
    fWindow = StringToDouble(GetParameter("window", fWindow));

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
}

void TRestRawPeaksFinderProcess::PrintMetadata() {
    cout << "Applying process to channel types: ";
    for (const auto& type : fChannelTypes) {
        cout << type << " ";
    }
    cout << endl;

    cout << "Applying process to channel ids: ";
    for (const auto& id : fChannelIds) {
        cout << id << " ";
    }
    cout << endl;

    cout << "Threshold over baseline: " << fThresholdOverBaseline << endl;
    cout << "Baseline range: " << fBaselineRange.X() << " - " << fBaselineRange.Y() << endl;

    cout << "Distance: " << fDistance << endl;
    cout << "Window: " << fWindow << endl;
}
