
#include <Rtypes.h>
#include <TClass.h>

class ASICMetadata {
  public:
    UShort_t polarity;
    UShort_t pedCenter;
    Float_t pedThr;
    UShort_t gain;
    UShort_t shappingTime;
    UShort_t channelStart;
    UShort_t channelEnd;
    Bool_t isActive;
    Bool_t channelActive[79];

  ASICMetadata();
  ClassDef(ASICMetadata,1);
};

