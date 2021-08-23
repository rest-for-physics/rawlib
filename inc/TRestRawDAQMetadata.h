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

  enum class acqTypes : int {
    BACKGROUND,
    CALIBRATION,
    PEDESTAL
  };
  
  enum class electronicsTypes : int {
    DUMMY,
    DCC,
    FEMINOS
  };

  enum class chipTypes : int {
    AFTER,
    AGET
  };

  const std::map<std::string, acqTypes> acqTypes_map = {{"background",acqTypes::BACKGROUND}, {"calibration",acqTypes::CALIBRATION},{"pedestal",acqTypes::PEDESTAL}};
  const std::map<std::string, electronicsTypes> electronicsTypes_map = {{"DUMMY",electronicsTypes::DUMMY}, {"DCC",electronicsTypes::DCC}, {"FEMINOS",electronicsTypes::FEMINOS}};
  const std::map<std::string, chipTypes> chipTypes_map = {{"after",chipTypes::AFTER}, {"aget",chipTypes::AGET}};


}

//! A metadata class to store DAQ information.
class TRestRawDAQMetadata : public TRestMetadata {
   private:
    void InitFromConfigFile();

    virtual void Initialize();

   protected:
    TString fElectronicsType;            // DCC, FEMINOS, ARC, ...
    TString fChipType;                   // after or aget
    UInt_t fFECMask;                     // FEC Mask
    UInt_t fGain;                        // Gain in the AFTER/AGET chip
    UInt_t fShappingTime;                // Shapping time in the AFTER/AGET chip
    UInt_t fClockDiv;                    // Clock division
    Int_t fBaseIp[4] = {192,168,10,13};  //Base IP of the card
    Int_t fLocalIp[4] = {192,168,10,10}; //Local IP of the host computer
    TString fTriggerType;                // external or internal
    TString fAcquisitionType;            // pedestal, calibration or background
    UInt_t fCompressMode =0;             // 0 uncompressed, 1 compress
    Int_t fNEvents=0;                    // 0 --> Infinite
    Int_t fPolarity =0;                  // 0--> negative, 1 positive

   public:
    virtual void PrintMetadata();
    void ReadIp(const std::string &param, Int_t *ip );

    // Construtor
    TRestRawDAQMetadata();
    TRestRawDAQMetadata(const char* cfgFileName);
    // Destructor
    virtual ~TRestRawDAQMetadata();

    UInt_t GetFECMask() { return fFECMask; }
    UInt_t GetGain() { return fGain; }
    UInt_t GetShappingTime() { return fShappingTime; }
    UInt_t GetClockDiv() { return fClockDiv; }
    TString GetTriggerType() { return fTriggerType; }
    TString GetAcquisitionType() { return fAcquisitionType; }
    TString GetElectronicsType() { return fElectronicsType; }
    TString GetChipType() { return fChipType; }
    Int_t GetNEvents() { return fNEvents; }
    Int_t* GetBaseIp() { return fBaseIp; }
    Int_t* GetLocalIp() { return fLocalIp; }
    UInt_t GetCompressMode(){return fCompressMode;}       // 0 uncompressed, 1 compress
    UInt_t GetPolarity(){return fPolarity;}
    UInt_t GetValFromString(TString var, TString line);
    TString GetDecodingFile(){return GetParameter("decodingFile");}

    void SetAcquisitionType (const std::string &typ){fAcquisitionType = typ;}
    void SetNEvents(const Int_t & nEv){fNEvents=nEv;}

    ClassDef(TRestRawDAQMetadata, 2);  // REST run class
};
#endif
