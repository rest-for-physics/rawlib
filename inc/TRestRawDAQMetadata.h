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
    TString fOutBinFileName;
    TString fElectronicsType;
    std::vector<TString> fPedBuffer;  // Pedestal script
    std::vector<TString> fRunBuffer;  // Run script
    TString fNamePedScript;           // Name of the run script e.g. /home/user/scripts/run
    TString fNameRunScript;           // Name of the pedestal script e.g.
                                      // /home/user/scripts/ped
    UInt_t fGain;                     // Value of the gain in the script you have to convert it to fC
    UInt_t fShappingTime;             // Value of the shapping time in the script you have to
                                      // convert it to nS

   public:
    void PrintMetadata();
    void PrintRunScript();
    void PrintPedScript();

    // Constructor
    TRestRawDAQMetadata();
    TRestRawDAQMetadata(char* configFilename);
    // Destructor
    virtual ~TRestRawDAQMetadata();

    void SetScriptsBuffer();
    void SetParFromPedBuffer();  // Set gain and shaping time from a given buffer
    void SetOutBinFileName(TString fName) { fOutBinFileName = fName; }

   inline UInt_t GetGain() const { return fGain; }
   inline UInt_t GetShappingTime() const { return fShappingTime; }
    UInt_t GetValFromString(TString var, TString line);

    ClassDef(TRestRawDAQMetadata, 1);  // REST run class
};
#endif
