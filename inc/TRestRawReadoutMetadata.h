//
// Created by lobis on 24-Aug-23.
//

#ifndef REST_TRESTRAWREADOUTMETADATA_H
#define REST_TRESTRAWREADOUTMETADATA_H

#include <TRestMetadata.h>

class TRestDetectorReadout;

class TRestRawReadoutMetadata : public TRestMetadata {
   public:
    struct ChannelInfo {
        std::string type;
        std::string name;
        UShort_t channelId;
        // other members
    };
    // maps daq id to channel info
    std::map<UShort_t, ChannelInfo> fChannelInfo;

    void InitializeFromReadout(TRestDetectorReadout* readout);

   public:
    std::string GetTypeForChannelDaqId(UShort_t channel) const;
    std::string GetNameForChannelDaqId(UShort_t channel) const;

    Int_t GetChannelIdForChannelDaqId(UShort_t channel) const;

    std::vector<UShort_t> GetChannelDaqIDsForType(const std::string& type) const;

    void PrintMetadata() const;

    TRestRawReadoutMetadata() = default;
    ~TRestRawReadoutMetadata() = default;

    static TRestRawReadoutMetadata* Metadata;

    ClassDef(TRestRawReadoutMetadata, 1)
};

#endif  // REST_TRESTRAWREADOUTMETADATA_H
