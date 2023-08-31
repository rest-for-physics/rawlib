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

std::string TRestRawReadoutMetadata::GetTypeForChannelId(UShort_t channel) const {
    if (fChannelInfo.find(channel) == fChannelInfo.end()) {
        cerr << "TRestRawReadoutMetadata::GetTypeForChannelId: channel " << channel << " not found" << endl;
        return {};
    }
    return fChannelInfo.at(channel).type;
}

std::string TRestRawReadoutMetadata::GetNameForChannelId(UShort_t channel) const {
    if (fChannelInfo.find(channel) == fChannelInfo.end()) {
        cerr << "TRestRawReadoutMetadata::GetNameForChannelId: channel " << channel << " not found" << endl;
        return {};
    }
    return fChannelInfo.at(channel).name;
}

std::vector<UShort_t> TRestRawReadoutMetadata::GetChannelIDsForType(const std::string& type) const {
    std::vector<UShort_t> result;
    for (const auto& channel : fChannelInfo) {
        if (channel.second.type == type) {
            result.push_back(channel.first);
        }
    }
    return result;
}
