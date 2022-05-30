
#ifndef RestCore_FECMetadata
#define RestCore_FECMetadata

#include "TString.h"

class FECMetadata {
  public:
    Int_t id;
    Int_t ip[4];
    UShort_t clockDiv;
    TString chipType;
    UShort_t asic_polarity[4];
    UShort_t asic_pedCenter[4];
    Float_t asic_pedThr[4];
    UShort_t asic_gain[4];
    UShort_t asic_shappingTime[4];
    UShort_t asic_channelStart[4];
    UShort_t asic_channelEnd[4];
    Bool_t asic_isActive[4];
    Bool_t asic_channelActive[4][79];

  FECMetadata();

  bool operator < (const FECMetadata &fM) const {
   return id < fM.id;
 }

  ClassDef(FECMetadata,1);
};

#endif
