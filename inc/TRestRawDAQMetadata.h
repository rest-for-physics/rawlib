/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

#ifndef RestCore_TRestRawDAQMetadata
#define RestCore_TRestRawDAQMetadata

#include <iostream>

#include "TRestMetadata.h"
#include "TString.h"

namespace daq_metadata_types {

enum class acqTypes : int { BACKGROUND = 0, CALIBRATION, PEDESTAL };

enum class electronicsTypes : int { DUMMY = 0, DCC, FEMINOS, ARC };

enum class chipTypes : int { AFTER = 0, AGET };

enum class triggerTypes : int { INTERNAL = 0, EXTERNAL, AUTO, TCM };

enum class compressModeTypes : int { ALLCHANNELS = 0, TRIGGEREDCHANNELS, ZEROSUPPRESSION };

const std::map<std::string, acqTypes> acqTypes_map = {{"background", acqTypes::BACKGROUND},
                                                      {"calibration", acqTypes::CALIBRATION},
                                                      {"pedestal", acqTypes::PEDESTAL}};

const std::map<std::string, electronicsTypes> electronicsTypes_map = {{"DUMMY", electronicsTypes::DUMMY},
                                                                      {"DCC", electronicsTypes::DCC},
                                                                      {"FEMINOS", electronicsTypes::FEMINOS},
{"ARC", electronicsTypes::ARC}                                      };

const std::map<std::string, chipTypes> chipTypes_map = {{"after", chipTypes::AFTER},
                                                        {"aget", chipTypes::AGET}};

const std::map<std::string, triggerTypes> triggerTypes_map = {{"internal", triggerTypes::INTERNAL},
                                                              {"external", triggerTypes::EXTERNAL},
                                                              {"auto", triggerTypes::AUTO},
                                                              {"tcm", triggerTypes::TCM}};

const std::map<std::string, compressModeTypes> compressMode_map = {
    {"allchannels", compressModeTypes::ALLCHANNELS},
    {"triggeredchannels", compressModeTypes::TRIGGEREDCHANNELS},
    {"zerosuppression", compressModeTypes::ZEROSUPPRESSION},
};

}  // namespace daq_metadata_types

//! A metadata class to store DAQ information.
class TRestRawDAQMetadata : public TRestMetadata {
   public:
    static constexpr int nAsics = 4;
    static constexpr int nChannels = 79;

    struct FECMetadata {
        Int_t id;
        Int_t ip[nAsics];
        UShort_t clockDiv;
        TString chipType;
        UShort_t triggerDelay = 0;
        std::array<UShort_t, nAsics> asic_polarity;
        std::array<UShort_t, nAsics> asic_pedCenter;
        std::array<Float_t, nAsics> asic_pedThr;
        std::array<UShort_t, nAsics> asic_gain;
        std::array<UShort_t, nAsics> asic_shappingTime;
        std::array<UShort_t, nAsics> asic_channelStart;
        std::array<UShort_t, nAsics> asic_channelEnd;
        std::array<Bool_t, nAsics> asic_isActive;
        std::array<UShort_t, nAsics> asic_coarseThr;
        std::array<UShort_t, nAsics> asic_fineThr;
        std::array<UShort_t, nAsics> asic_multThr;
        std::array<UShort_t, nAsics> asic_multLimit;
        std::array<std::array<Bool_t, nAsics>, nChannels> asic_channelActive;

        bool operator<(const FECMetadata& fM) const { return id < fM.id; }
        void operator=(const FECMetadata& fM) {
            id = fM.id;
            clockDiv = fM.clockDiv;
            chipType = fM.chipType;
            for (int i = 0; i < nAsics; i++) {
                ip[i] = fM.ip[i];
                asic_polarity[i] = fM.asic_polarity[i];
                asic_pedCenter[i] = fM.asic_pedCenter[i];
                asic_pedThr[i] = fM.asic_pedThr[i];
                asic_gain[i] = fM.asic_gain[i];
                asic_shappingTime[i] = fM.asic_shappingTime[i];
                asic_channelStart[i] = fM.asic_channelStart[i];
                asic_channelEnd[i] = fM.asic_channelEnd[i];
                asic_isActive[i] = fM.asic_isActive[i];
                asic_coarseThr[i] = fM.asic_coarseThr[i];
                asic_fineThr[i] = fM.asic_fineThr[i];
                asic_multThr[i] = fM.asic_multThr[i];
                asic_multLimit[i] = fM.asic_multLimit[i];
                for (int j = 0; j < nChannels; j++) {
                    asic_channelActive[i][j] = fM.asic_channelActive[i][j];
                }
            }
        }
    };

   private:
    void InitFromConfigFile() override;

    void Initialize() override;

   protected:
    TString fElectronicsType;         // DCC, FEMINOS, ARC, ...
    TString fTriggerType;             // external, internal, auto or tcm
    TString fAcquisitionType;         // pedestal, calibration or background
    TString fCompressMode;            // allchannels, triggeredchannels, zerosuppression
    Int_t fNEvents = 0;               // 0 --> Infinite
    Int_t fNPedestalEvents = 100;     // Number of pedestal events to be acquired
    std::vector<FECMetadata> fFEC;    // Vector of FECMETADATA
    TString fDecodingFile = "";       // Location of the decoding file
    Int_t fMaxFileSize = 1000000000;  // Maximum file size in bytes

   public:
    void PrintMetadata() override;

    // Constructor
    TRestRawDAQMetadata();
    TRestRawDAQMetadata(const char* configFilename);
    // Destructor
    virtual ~TRestRawDAQMetadata();

    inline auto GetTriggerType() const { return fTriggerType; }
    inline auto GetAcquisitionType() const { return fAcquisitionType; }
    inline auto GetElectronicsType() const { return fElectronicsType; }
    inline auto GetNEvents() const { return fNEvents; }
    inline auto GetNPedestalEvents() const { return fNPedestalEvents; }
    inline auto GetCompressMode() const { return fCompressMode; }
    inline auto GetDecodingFile() const { return fDecodingFile; }
    inline auto GetFECs() const { return fFEC; }
    inline auto GetMaxFileSize() const { return fMaxFileSize; }

    void SetAcquisitionType(const std::string& typ) { fAcquisitionType = typ; }
    void SetNEvents(const Int_t& nEv) { fNEvents = nEv; }

    void ReadFEC();
    void DumpFEC(const FECMetadata& fec);

    ClassDefOverride(TRestRawDAQMetadata, 3);  // REST run class
};
#endif
