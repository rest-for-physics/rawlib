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
    TRestRawSignalEvent* fInputEvent = nullptr;           //!
    TRestRawReadoutMetadata* fReadoutMetadata = nullptr;  //!

    /// \brief threshold over baseline to consider a peak
    Double_t fThresholdOverBaseline = 2.0;
    /// \brief choose times the sigma of the baseline must be overcome to consider a peak
    Double_t fSigmaOverBaseline = 10.0;
    /// \brief range of samples to calculate baseline for peak finding
    TVector2 fBaselineRange = {0, 10};
    /// \brief distance between two peaks to consider them as different (ADC units)
    UShort_t fDistance = 10;
    /// \brief window size to calculate the peak amplitude (time bins)
    UShort_t fWindow = 10;
    /// \brief option to remove all veto signals after finding the peaks
    Bool_t fRemoveAllVetoes = false;
    /// \brief option to remove peak-less veto signals after finding the peaks
    Bool_t fRemovePeaklessVetoes = false;

    Double_t fTimeBinToTimeFactorMultiplier = 0.0;
    Double_t fTimeBinToTimeFactorOffset = 0.0;
    Double_t fTimeBinToTimeFactorOffsetTCM = 0.0;

    Bool_t fTimeConversionElectronics = false;

    Double_t fADCtoEnergyFactor = 0.0;
    std::map<UShort_t, Double_t> fChannelIDToADCtoEnergyFactor = {};

    std::set<std::string> fChannelTypes = {};  // this process will only be applied to selected channel types

   public:
    RESTValue GetInputEvent() const override { return fInputEvent; }
    RESTValue GetOutputEvent() const override { return fInputEvent; }

    void PrintMetadata() override;

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override {}

    const char* GetProcessName() const override { return "peaksFinder"; }

    explicit TRestRawPeaksFinderProcess(const char* configFilename){};

    void InitFromConfigFile() override;

    TRestRawPeaksFinderProcess() = default;
    ~TRestRawPeaksFinderProcess() = default;

    ClassDefOverride(TRestRawPeaksFinderProcess, 6);
};

#endif  // REST_TRESTRAWPEAKSFINDERPROCESS_H
