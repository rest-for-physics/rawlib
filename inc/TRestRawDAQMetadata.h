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

//! A metadata class to store DAQ information.
class TRestRawDAQMetadata : public TRestMetadata {
   private:
    void InitFromConfigFile();

    virtual void Initialize();

   protected:
    TString fElectronicsType;         // DCC, FEMINOS, ARC, ...

    UInt_t fFECMask;                  // FEC Mask
    UInt_t fGain;                     // Gain in the AFTER/AGET chip
    UInt_t fShappingTime;             // Shapping time in the AFTER/AGET chip
    UInt_t fClockDiv;                 // Clock division
    Int_t fBaseIp[4] = {192, 168,10,13}; //Base IP of the card
    TString fTriggerType;             // external or internal
    TString fAcquisitionType;         // pedestal, calibration or background
    UInt_t fCompressMode =0;             // 0 uncompressed, 1 compress
    UInt_t fNEvents=0;                  // 0 --> Infinite

   public:
    virtual void PrintMetadata();
    void ReadBaseIp();

    // Construtor
    TRestRawDAQMetadata();
    TRestRawDAQMetadata(char* cfgFileName);
    // Destructor
    virtual ~TRestRawDAQMetadata();

    UInt_t GetFECMask() { return fFECMask; }
    UInt_t GetGain() { return fGain; }
    UInt_t GetShappingTime() { return fShappingTime; }
    UInt_t GetClockDiv() { return fClockDiv; }
    TString GetTriggerType() { return fTriggerType; }
    TString GetAcquisitionType() { return fAcquisitionType; }
    UInt_t GetNEvents() { return fNEvents; }
    Int_t* GetBaseIp() { return fBaseIp; }
    UInt_t GetCompressMode(){return fCompressMode;}       // 0 uncompressed, 1 compress
    
    UInt_t GetValFromString(TString var, TString line);

    ClassDef(TRestRawDAQMetadata, 2);  // REST run class
};
#endif
