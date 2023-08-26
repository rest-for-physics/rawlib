//
// Created by lobis on 24-Aug-23.
//

#ifndef REST_TRESTRAWPEAKSFINDERPROCESS_H
#define REST_TRESTRAWPEAKSFINDERPROCESS_H

#include <TRestEventProcess.h>

#include "TRestRawReadoutMetadata.h"
#include "TRestRawSignalEvent.h"

class TRestRawPeaksFinderProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fSignalEvent = nullptr;          //!
    TRestRawReadoutMetadata* fReadoutMetadata = nullptr;  //!

    std::set<std::string> fChannelTypes = {};  // this process will only be applied to selected channel types

    std::set<UShort_t> fChannelIds;  // this process will only be applied to selected channel ids

   public:
    any GetInputEvent() const override { return fSignalEvent; }
    any GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override {}

    const char* GetProcessName() const override { return "peaksFinder"; }

    explicit TRestRawPeaksFinderProcess(const char* configFilename){};

    void InitFromConfigFile() override;

    static TRestRawReadoutMetadata* Metadata;

    TRestRawPeaksFinderProcess() = default;
    ~TRestRawPeaksFinderProcess() = default;

    ClassDefOverride(TRestRawPeaksFinderProcess, 2);
};

#endif  // REST_TRESTRAWPEAKSFINDERPROCESS_H
