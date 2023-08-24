//
// Created by lobis on 24-Aug-23.
//

#include "TRestRawPeaksFinderProcess.h"

#include "TRestRawReadoutMetadata.h"

ClassImp(TRestRawPeaksFinderProcess);

using namespace std;

TRestRawReadoutMetadata* TRestRawPeaksFinderProcess::Metadata = nullptr;

void TRestRawPeaksFinderProcess::InitProcess() {
    // fReadoutMetadata = GetMetadata<TRestRawReadoutMetadata>();

    fReadoutMetadata = TRestRawPeaksFinderProcess::Metadata;

    if (!fReadoutMetadata) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: raw readout metadata is null" << endl;
        exit(1);
    }
}

TRestEvent* TRestRawPeaksFinderProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    for (int signalIndex = 0; signalIndex < fSignalEvent->GetNumberOfSignals(); signalIndex++) {
        const auto signal = fSignalEvent->GetSignal(signalIndex);
        const UShort_t channelId = signal->GetSignalID();
        const string channelType = fReadoutMetadata->GetChannelType(channelId);
        const string channelName = fReadoutMetadata->GetChannelName(channelId);

        // check if channel type is in the list of selected channel types
        if (fChannelTypes.find(channelType) == fChannelTypes.end()) {
            continue;
        }

        signal->CalculateBaseLine(0, 5);
        const auto peaks = signal->GetPeaks(signal->GetBaseLine() + 1.0);

        cout << "Signal ID: " << channelId << " Name: " << channelName << endl;
        for (const auto& [time, amplitude] : peaks) {
            cout << "   - Peak at " << time << " with amplitude " << amplitude << endl;
        }
    }

    return inputEvent;
}
