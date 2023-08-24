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
        // other members
    };
    std::map<UShort_t, ChannelInfo> fChannelInfo;

    void InitializeFromReadout(TRestDetectorReadout* readout);

   public:
    bool IsChannelType(UShort_t channel, std::string type) const;
    bool IsChannelVeto(UShort_t channel) const;
    bool IsChannelMicromegas(UShort_t channel) const;
    std::string GetChannelType(UShort_t channel) const;
    std::string GetChannelName(UShort_t channel) const;

    void PrintMetadata() const;

    TRestRawReadoutMetadata() = default;
    ~TRestRawReadoutMetadata() = default;

    ClassDef(TRestRawReadoutMetadata, 1)
};

#endif  // REST_TRESTRAWREADOUTMETADATA_H
