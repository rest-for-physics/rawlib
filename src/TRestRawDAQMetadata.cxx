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

//////////////////////////////////////////////////////////////////////////
/// TRestRawDAQMetadata class is meant to hold DAQ information which is
/// stored in the root files generated using `restDAQ` package. It contains
/// information about the readout type DCC, FEMINOS,... and the different
/// parameters of the readout electronics (FEC, FEM, AGET, AFTER,...). Note
/// that the DAQ is under development, so further changes on this class are
/// foreseen:
///
/// ### Parameters
/// Describe any parameters this process receives:
/// * **electronicsType**: DAQ electronics type, only DCC and FEMINOS are
/// supported so far.
/// * **triggerType**: internal, external, auto or tcm
/// * **acquisitionType**: Type of acquisition: pedestal, calibration or
/// background
/// * **compressMode**: allchannels, triggeredchannels or zerosuppression
/// * **nEvents**: Number of events to be acquired (0 for infinite loop)
/// * **nPedestalEvents**: Number of pedestal events to be acquired
/// It also includes `FECMetadata` section (see example)
///
/// ### Examples
/// Give examples of usage and RML descriptions that can be tested.
/// \code
/// <TRestRawDAQMetadata name="DAQMetadata" title="DAQ Metadata" verboseLevel="info">
///         <parameter name ="electronicsType" value="DUMMY"/>
///         <parameter name ="triggerType" value="external"/>
///         <parameter name ="acquisitionType" value="background"/>
///         <parameter name ="compressMode" value="zerosuppression"/>
///         <parameter name ="Nevents" value="0"/>
///         <parameter name ="nPedestalEvents" value="100"/>
///
///   <FEC id="2" ip="192:168:10:13" chip="after" clockDiv="0x2">
///     <ASIC id="*" isActive="true" gain="0x1" shappingTime="0x2" polarity="0" pedcenter="250" pedthr="5.0"
///     coarseThr="0x2" fineThr="0x7" multThr="32" multLimit="232">
///         <channel id="*" isActive="true"></channel>
///         <channel id="0" isActive="false"></channel>
///         <channel id="1" isActive="false"></channel>
///         <channel id="2" isActive="false"></channel>
///     </ASIC>
///   </FEC>
/// </TRestRawDAQMetadata>
/// \endcode
///
///----------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2015-Nov: First implementation as part of the conceptualization of existing
///           REST software.
/// JuanAn Garcia
///
/// 2021-2022: New implementation for restDAQ package
/// JuanAn Garcia
///
/// \class TRestRawDAQMetadata
/// \author: Juanan Garcia e-mail: juanangp@unizar.es
///
/// <hr>
///
#include "TRestRawDAQMetadata.h"
using namespace std;

ClassImp(TRestRawDAQMetadata);

TRestRawDAQMetadata::TRestRawDAQMetadata() { Initialize(); }

TRestRawDAQMetadata::TRestRawDAQMetadata(const char* configFilename) : TRestMetadata(configFilename) {
    Initialize();

    LoadConfigFromFile(fConfigFileName);
}

void TRestRawDAQMetadata::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
}

TRestRawDAQMetadata::~TRestRawDAQMetadata() {}

void TRestRawDAQMetadata::InitFromConfigFile() {
    TRestMetadata::InitFromConfigFile();

    ReadFEC();
}

void TRestRawDAQMetadata::PrintMetadata() {
    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
    RESTMetadata << this->ClassName() << " content" << RESTendl;
    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
    RESTMetadata << "Trigger type : " << fTriggerType.Data() << RESTendl;
    RESTMetadata << "Acquisition type : " << fAcquisitionType.Data() << RESTendl;
    RESTMetadata << "Compress mode : " << fCompressMode.Data() << RESTendl;
    RESTMetadata << "Number of events : " << fNEvents << RESTendl;
    RESTMetadata << "Number of pedestal events : " << fNPedestalEvents << RESTendl;
    RESTMetadata << "Maximum file size : " << fMaxFileSize << " bytes" << RESTendl;

    for (const auto& f : fFEC) DumpFEC(f);
    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;

    RESTMetadata << RESTendl;
}

void TRestRawDAQMetadata::ReadFEC() {
    TiXmlElement* FECDef = GetElement("FEC");

    while (FECDef) {
        FECMetadata fec;
        fec.id = StringToInteger(GetFieldValue("id", FECDef));
        std::string ip = GetFieldValue("ip", FECDef);
        sscanf(ip.c_str(), "%d:%d:%d:%d", &fec.ip[0], &fec.ip[1], &fec.ip[2], &fec.ip[3]);
        fec.chipType = GetFieldValue("chip", FECDef);
        fec.clockDiv = StringToInteger(GetFieldValue("clockDiv", FECDef));
        // std::cout<<"FEC "<<fec.id<<" ip:"<<fec.ip[0]<<"."<<fec.ip[1]<<"."<<fec.ip[2]<<"."<<fec.ip[3]<<"
        // "<<ip<<" "<<fec.chipType<<" clockDiv "<<fec.clockDiv<<std::endl;

        TiXmlElement* ASICDef = GetElement("ASIC", FECDef);
        // std::cout<<"ASICDef "<<ASICDef<<std::endl;
        while (ASICDef) {
            std::string id = GetFieldValue("id", ASICDef);
            int gain = StringToInteger(GetFieldValue("gain", ASICDef));
            int shappingTime = StringToInteger(GetFieldValue("shappingTime", ASICDef));
            bool asicActive = StringToBool(GetFieldValue("isActive", ASICDef));
            // std::cout<<"ASIC "<<id<<" gain "<<gain<<" shappingTime "<<shappingTime<<std::endl;
            uint16_t polarity = StringToInteger(GetFieldValue("polarity", ASICDef)) & 0x1;
            uint16_t pedCenter = StringToInteger(GetFieldValue("pedcenter", ASICDef));
            float pedThr = StringToFloat(GetFieldValue("pedthr", ASICDef));
            uint16_t coarseThr = StringToInteger(GetFieldValue("coarseThr", ASICDef));
            uint16_t fineThr = StringToInteger(GetFieldValue("fineThr", ASICDef));
            uint16_t multThr = StringToInteger(GetFieldValue("multThr", ASICDef));
            uint16_t multLimit = StringToInteger(GetFieldValue("multLimit", ASICDef));

            TiXmlElement* channelDef = GetElement("channel", ASICDef);
            bool channelActive[nChannels];
            while (channelDef) {
                std::string chId = GetFieldValue("id", channelDef);
                bool active = StringToBool(GetFieldValue("isActive", channelDef));
                if (chId == "*") {
                    for (int i = 0; i < nChannels; i++) channelActive[i] = active;
                } else {
                    int i = StringToInteger(chId);
                    if (i < nChannels) channelActive[i] = active;
                }
                channelDef = GetNextElement(channelDef);
            }

            if (id == "*") {  // Wildcard
                for (int i = 0; i < nAsics; i++) {
                    fec.asic_gain[i] = gain;
                    fec.asic_shappingTime[i] = shappingTime;
                    fec.asic_isActive[i] = asicActive;
                    fec.asic_polarity[i] = polarity;
                    fec.asic_pedCenter[i] = pedCenter;
                    fec.asic_pedThr[i] = pedThr;
                    fec.asic_coarseThr[i] = coarseThr;
                    fec.asic_fineThr[i] = fineThr;
                    fec.asic_multThr[i] = multThr;
                    fec.asic_multLimit[i] = multLimit;
                    bool isFirst = true;
                    for (int c = 0; c < nChannels; c++) {
                        fec.asic_channelActive[i][c] = channelActive[c];
                        if (channelActive[c] && isFirst) {
                            fec.asic_channelStart[i] = c;
                            isFirst = false;
                        }
                        if (channelActive[c]) fec.asic_channelEnd[i] = c;
                    }
                }
            } else {
                int i = StringToInteger(id);
                if (i < nAsics) {
                    fec.asic_gain[i] = gain;
                    fec.asic_shappingTime[i] = shappingTime;
                    fec.asic_isActive[i] = asicActive;
                    fec.asic_polarity[i] = polarity;
                    fec.asic_pedCenter[i] = pedCenter;
                    fec.asic_pedThr[i] = pedThr;
                    fec.asic_coarseThr[i] = coarseThr;
                    fec.asic_fineThr[i] = fineThr;
                    fec.asic_multThr[i] = multThr;
                    fec.asic_multLimit[i] = multLimit;
                    bool isFirst = true;
                    for (int c = 0; c < nChannels; c++) {
                        fec.asic_channelActive[i][c] = channelActive[c];
                        if (channelActive[c] && isFirst) {
                            fec.asic_channelStart[i] = c;
                            isFirst = false;
                        }
                        if (channelActive[c]) fec.asic_channelEnd[i] = c;
                    }
                }
            }
            ASICDef = GetNextElement(ASICDef);
        }

        fFEC.emplace_back(std::move(fec));
        FECDef = GetNextElement(FECDef);
    }

    std::sort(fFEC.begin(), fFEC.end());
}

void TRestRawDAQMetadata::DumpFEC(const FECMetadata& fec) {
    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
    RESTMetadata << "FEC id:" << fec.id << RESTendl;
    RESTMetadata << "IP: " << fec.ip[0] << "." << fec.ip[1] << "." << fec.ip[2] << "." << fec.ip[3]
                 << RESTendl;
    RESTMetadata << "Chip type: " << fec.chipType << RESTendl;
    RESTMetadata << "Clock Div: 0x" << std::hex << fec.clockDiv << std::dec << RESTendl;
    for (int i = 0; i < nAsics; i++) {
        if (!fec.asic_isActive[i]) continue;
        RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
        RESTMetadata << "ASIC " << i << RESTendl;
        RESTMetadata << "Polarity: (AGET) " << fec.asic_polarity[i] << RESTendl;
        RESTMetadata << "Gain: 0x" << std::hex << fec.asic_gain[i] << std::dec << RESTendl;
        RESTMetadata << "ShappingTime: 0x" << std::hex << fec.asic_shappingTime[i] << std::dec << RESTendl;
        RESTMetadata << "Channel start: " << fec.asic_channelStart[i] << RESTendl;
        RESTMetadata << "Channel end: " << fec.asic_channelEnd[i] << RESTendl;
        RESTMetadata << "Coarse threshold (AGET): 0x" << std::hex << fec.asic_coarseThr[i] << std::dec
                     << RESTendl;
        RESTMetadata << "Fine threshold (AGET): 0x" << std::hex << fec.asic_fineThr[i] << std::dec
                     << RESTendl;
        RESTMetadata << "Multiplicity threshold (AGET): " << fec.asic_multThr[i] << RESTendl;
        RESTMetadata << "Multiplicity limit (AGET): " << fec.asic_multLimit[i] << RESTendl;
        RESTMetadata << "Active channels: " << RESTendl;
        for (int c = 0; c < nChannels; c++) {
            if (fec.asic_channelActive[i][c]) RESTMetadata << c << "; ";
            if (c > 0 && c % 10 == 0) RESTMetadata << RESTendl;
        }
        RESTMetadata << RESTendl;
    }
    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
}
