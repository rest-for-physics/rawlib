//
// Created by lobis on 24-Aug-23.
//

#include "TRestRawPeaksFinderProcess.h"

ClassImp(TRestRawPeaksFinderProcess);

using namespace std;

void TRestRawPeaksFinderProcess::InitProcess() {
    fReadoutMetadata = GetMetadata<TRestRawReadoutMetadata>();

    if (!fReadoutMetadata) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: raw readout metadata is null" << endl;
        exit(1);
    }
}

TRestEvent* TRestRawPeaksFinderProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    for (int signalIndex = 0; signalIndex < fSignalEvent->GetNumberOfSignals(); signalIndex++) {
        const auto signal = fSignalEvent->GetSignal(signalIndex);
        const auto channelId = signal->GetSignalID();
        const auto channelType = fReadoutMetadata->GetChannelType(channelId);

        // check if channel type is in the list of selected channel types
        if (fChannelTypes.find(channelType) == fChannelTypes.end()) {
            continue;
        }

        // const auto peaks = signal->GetPeaks();
    }

    return inputEvent;
}
