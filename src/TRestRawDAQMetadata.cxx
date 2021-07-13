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
/// The TRestRawDaqMetadata ...
///
/// TODO. This class might be obsolete today. It may need additional revision,
/// validation, and documentation.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2015-Nov: First implementation as part of the conceptualization of existing
///			  REST software.
///           Juanan Garcia
///
/// \class      TRestRawDAQMetadata
/// \author     Juanan Garcia
///
/// <hr>
///
#include "TRestRawDAQMetadata.h"
using namespace std;

ClassImp(TRestRawDAQMetadata);
//______________________________________________________________________________
TRestRawDAQMetadata::TRestRawDAQMetadata() { Initialize(); }

TRestRawDAQMetadata::TRestRawDAQMetadata(char* cfgFileName) : TRestMetadata(cfgFileName) {
    Initialize();

    LoadConfigFromFile(fConfigFileName);
}

void TRestRawDAQMetadata::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
}

//______________________________________________________________________________
TRestRawDAQMetadata::~TRestRawDAQMetadata() { 

 }

//______________________________________________________________________________
void TRestRawDAQMetadata::InitFromConfigFile() {
    // string daqString;

    fElectronicsType = GetParameter("electronics");
    fFECMask = StringToInteger( GetParameter("fecMask") );
    fGain = StringToInteger( GetParameter("chipGain") );
    fShappingTime = StringToInteger( GetParameter("chipShappingTime") );
    fClockDiv = StringToInteger( GetParameter("clockDiv") );
    fTriggerType = GetParameter("triggerType");
    fAcquisitionType = GetParameter("acquisitionType");
    fCompressMode = StringToInteger(GetParameter("compressMode"));
    fNEvents = StringToInteger(GetParameter("Nevents"));
    ReadBaseIp( );

}

void TRestRawDAQMetadata::ReadBaseIp( ){

  std::string ip = GetParameter("baseIp");
  sscanf(ip.c_str(),"%d.%d.%d.%d",&fBaseIp[3],&fBaseIp[2],&fBaseIp[1],&fBaseIp[0]);

}

void TRestRawDAQMetadata::PrintMetadata() {
    cout << endl;
    cout << "====================================" << endl;
    cout << "Base IP : " << fBaseIp[0]<<"."<< fBaseIp[1]<<"."<< fBaseIp[2]<<"."<< fBaseIp[3]<< endl;
    cout << "ElectronicsType : " << fElectronicsType.Data() << endl;
    cout << "FEC mask : 0x"<<std::hex << fFECMask <<std::dec<< endl;
    cout << "Gain : 0x"<<std::hex << fGain <<std::dec<< endl;
    cout << "Shapping time 0x: " <<std::hex << fShappingTime <<std::dec<< endl;
    cout << "Clock div 0x: " <<std::hex << fClockDiv <<std::dec<< endl;
    cout << "Trigger type : " << fTriggerType.Data() << endl;
    cout << "Acquisition type : " << fAcquisitionType.Data() << endl;
    cout << "Number of events : " << fNEvents << endl;
    cout << "====================================" << endl;
    cout << endl;
}




