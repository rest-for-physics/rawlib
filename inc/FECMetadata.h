
#include "ASICMetadata.h"
#include "TString.h"
#include <TClass.h>

class FECMetadata {
  public:
    Int_t id;
    Int_t ip[4];
    UShort_t clockDiv;
    TString chipType;
    ASICMetadata asic[4];

  FECMetadata();

  bool operator < (const FECMetadata &fM) const {
   return id < fM.id;
 }

  ClassDef(FECMetadata,1);
};

