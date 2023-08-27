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
        const auto peaks = signal->GetPeaks(signal->GetBaseLine() + 1.0);

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

    // sort eventPeaks by signal id, then by time
    sort(eventPeaks.begin(), eventPeaks.end(), [](const auto& a, const auto& b) {
        return get<0>(a) < get<0>(b) || (get<0>(a) == get<0>(b) && get<1>(a) < get<1>(b));
    });

    SetObservableValue("peaks", eventPeaks);

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

    if (fBaselineRange.X() > fBaselineRange.Y() || fBaselineRange.X() < 0 || fBaselineRange.Y() < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: baseline range is not sorted or < 0" << endl;
        exit(1);
    }

    if (fThresholdOverBaseline < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: threshold over baseline is < 0" << endl;
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
}
