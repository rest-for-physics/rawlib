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

    /// \brief threshold over baseline to consider a peak
    Double_t fThresholdOverBaseline = 2.0;
    /// \brief range of samples to calculate baseline for peak finding
    TVector2 fBaselineRange = {0, 10};
    /// \brief distance between two peaks to consider them as different
    UShort_t fDistance = 10;
    /// \brief window size to calculate the peak amplitude
    UShort_t fWindow = 10;

    std::set<std::string> fChannelTypes = {};  // this process will only be applied to selected channel types

   public:
    any GetInputEvent() const override { return fSignalEvent; }
    any GetOutputEvent() const override { return fSignalEvent; }

    void PrintMetadata() override;

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override {}

    const char* GetProcessName() const override { return "peaksFinder"; }

    explicit TRestRawPeaksFinderProcess(const char* configFilename){};

    void InitFromConfigFile() override;

    TRestRawPeaksFinderProcess() = default;
    ~TRestRawPeaksFinderProcess() = default;

    ClassDefOverride(TRestRawPeaksFinderProcess, 3);
};

#endif  // REST_TRESTRAWPEAKSFINDERPROCESS_H
