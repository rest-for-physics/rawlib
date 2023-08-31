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
        UShort_t daqId;
        // other members
    };
    // maps daq id to channel info
    std::map<UShort_t, ChannelInfo> fChannelInfo;

    void InitializeFromReadout(TRestDetectorReadout* readout);

   public:
    std::string GetTypeForChannelId(UShort_t channel) const;
    std::string GetNameForChannelId(UShort_t channel) const;

    Int_t GetDaqIdForChannelId(UShort_t channel) const;

    std::vector<UShort_t> GetChannelIDsForType(const std::string& type) const;

    void PrintMetadata() const;

    TRestRawReadoutMetadata() = default;
    ~TRestRawReadoutMetadata() = default;

    ClassDef(TRestRawReadoutMetadata, 1)
};

#endif  // REST_TRESTRAWREADOUTMETADATA_H
