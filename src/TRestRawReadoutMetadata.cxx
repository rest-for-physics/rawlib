//
// Created by lobis on 24-Aug-23.
//

#include "TRestRawReadoutMetadata.h"

using namespace std;

ClassImp(TRestRawReadoutMetadata);

void TRestRawReadoutMetadata::PrintMetadata() const {
    cout << "Number of channels: " << fChannelInfo.size() << endl;
    map<string, int> typesCount;
    for (const auto& channel : fChannelInfo) {
        const auto& info = channel.second;
        typesCount[info.type]++;
    }
    cout << "Channel types:" << endl;
    for (const auto& type : typesCount) {
        cout << type.first << ": " << type.second << endl;
    }

    for (const auto& [channelId, info] : fChannelInfo) {
        cout << "Channel " << channelId << ": " << info.type << " " << info.name << endl;
    }
}
