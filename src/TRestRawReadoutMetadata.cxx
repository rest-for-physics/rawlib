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
    cout << "Channel types: ";
    for (const auto& type : typesCount) {
        cout << type.first << " (" << type.second << "), ";
    }
    cout << endl;

    for (const auto& [channelId, info] : fChannelInfo) {
        cout << "   - Channel " << channelId << ": " << info.type << " " << info.name << endl;
    }
}

std::string TRestRawReadoutMetadata::GetChannelType(UShort_t channel) const {
    if (fChannelInfo.find(channel) == fChannelInfo.end()) {
        cerr << "TRestRawReadoutMetadata::GetChannelType: channel " << channel << " not found" << endl;
        return {};
    }
    return fChannelInfo.at(channel).type;
}

std::string TRestRawReadoutMetadata::GetChannelName(UShort_t channel) const {
    if (fChannelInfo.find(channel) == fChannelInfo.end()) {
        cerr << "TRestRawReadoutMetadata::GetChannelName: channel " << channel << " not found" << endl;
        return {};
    }
    return fChannelInfo.at(channel).name;
}
