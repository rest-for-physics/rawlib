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

TRestRawDAQMetadata::TRestRawDAQMetadata(const char* cfgFileName) : TRestMetadata(cfgFileName) {
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
    fChipType = GetParameter("chip");
    fClockDiv = StringToInteger( GetParameter("clockDiv") );
    fTriggerType = GetParameter("triggerType");
    fAcquisitionType = GetParameter("acquisitionType");
    fCompressMode = StringToInteger(GetParameter("compressMode"));
    fNEvents = StringToInteger(GetParameter("Nevents"));
    ReadIp("baseIp",fBaseIp);
    ReadIp("localIp",fLocalIp);
    ReadFEC();
}

void TRestRawDAQMetadata::ReadIp(const std::string &param, Int_t *ip ){

  std::string ipString = GetParameter(param);
  sscanf(ipString.c_str(),"%d:%d:%d:%d",&ip[0],&ip[1],&ip[2],&ip[3]);
  //std::cout<<param<<": "<<ip<<" --> "<<ip[0]<<" "<<ip[1]<<" "<<ip[2]<<" "<<ip[3]<<std::endl;
}

void TRestRawDAQMetadata::PrintMetadata() {
    metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
    metadata << this->ClassName() << " content" << endl;
    metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
    metadata << "Base IP : " << fBaseIp[0]<<"."<< fBaseIp[1]<<"."<< fBaseIp[2]<<"."<< fBaseIp[3]<< endl;
    metadata << "Local IP : " << fLocalIp[0]<<"."<< fLocalIp[1]<<"."<< fLocalIp[2]<<"."<< fLocalIp[3]<< endl;
    metadata << "ElectronicsType : " << fElectronicsType.Data() << endl;
    metadata << "Clock div : 0x" <<std::hex << fClockDiv <<std::dec<< endl;
    metadata << "Trigger type : " << fTriggerType.Data() << endl;
    metadata << "Acquisition type : " << fAcquisitionType.Data() << endl;
    metadata << "Number of events : " << fNEvents << endl;

      for(auto f : fFEC)DumpFEC(f);
    metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;

    cout << endl;
}

void TRestRawDAQMetadata::ReadFEC(){

  TiXmlElement* FECDef = GetElement("FEC");

  while(FECDef) {

    FECMetadata fec;
    fec.id = StringToInteger(GetFieldValue("id", FECDef) );
    std::string ip = GetFieldValue("ip", FECDef);
    sscanf(ip.c_str(),"%d:%d:%d:%d",&fec.ip[0],&fec.ip[1],&fec.ip[2],&fec.ip[3]);
    fec.chipType = GetFieldValue("chip", FECDef);
    fec.clockDiv = StringToInteger(GetFieldValue("clockDiv", FECDef) );
    //std::cout<<"FEC "<<fec.id<<" ip:"<<fec.ip[0]<<"."<<fec.ip[1]<<"."<<fec.ip[2]<<"."<<fec.ip[3]<<" "<<ip<<" "<<fec.chipType<<" clockDiv "<<fec.clockDiv<<std::endl;

    TiXmlElement* ASICDef = GetElement("ASIC",FECDef);
    //std::cout<<"ASICDef "<<ASICDef<<std::endl;
      while (ASICDef){
        std::string id = GetFieldValue("id", ASICDef);
        int gain = StringToInteger(GetFieldValue("gain", ASICDef) );
        int shappingTime = StringToInteger(GetFieldValue("shappingTime", ASICDef) );
        bool asicActive = StringToBool(GetFieldValue("isActive", ASICDef) );
        //std::cout<<"ASIC "<<id<<" gain "<<gain<<" shappingTime "<<shappingTime<<std::endl;
        uint16_t polarity = StringToInteger(GetFieldValue("polarity", ASICDef) ) & 0x1;
        uint16_t pedCenter = StringToInteger(GetFieldValue("pedcenter", ASICDef) );
        float pedThr = StringToFloat(GetFieldValue("pedthr", ASICDef) );
          TiXmlElement* channelDef = GetElement("channel",ASICDef);
          bool channelActive[79];
            while (channelDef){
              std::string chId = GetFieldValue("id", channelDef);
              bool active = StringToBool(GetFieldValue("isActive", channelDef) );
                if(chId=="*"){
                  for(int i=0;i<79;i++)channelActive[i]=active;
                } else {
                  int i= StringToInteger(chId);
                  if(i<79)
                    channelActive[i]=active;
                }
              channelDef = GetNextElement(channelDef);
            }

          if(id=="*"){//Wildcard
            for(int i=0;i<4;i++){
              fec.asic[i].gain = gain;
              fec.asic[i].shappingTime = shappingTime;
              fec.asic[i].isActive = asicActive;
              fec.asic[i].polarity = polarity;
              fec.asic[i].pedCenter = pedCenter;
              fec.asic[i].pedThr = pedThr;
              bool isFirst = true;
                for(int c=0;c<79;c++){
                  fec.asic[i].channelActive[c]=channelActive[c];
                    if(channelActive[c] && isFirst){
                      fec.asic[i].channelStart = c;
                      isFirst=false;
                    }
                    if(channelActive[c])fec.asic[i].channelEnd = c;
                }
            }
          } else {
            int i= StringToInteger(id);
            if(i<4){
              fec.asic[i].gain = gain;
              fec.asic[i].shappingTime = shappingTime;
              fec.asic[i].isActive = asicActive;
              fec.asic[i].polarity = polarity;
              fec.asic[i].pedCenter = pedCenter;
              fec.asic[i].pedThr = pedThr;
              bool isFirst = true;
                for(int c=0;c<79;c++){
                  fec.asic[i].channelActive[c]=channelActive[c];
                    if(channelActive[c] && isFirst){
                      fec.asic[i].channelStart = c;
                      isFirst=false;
                    }
                    if(channelActive[c])fec.asic[i].channelEnd = c;
                }
            }
          }
        ASICDef = GetNextElement(ASICDef);
      }

   fFEC.emplace_back( std::move(fec) );
   FECDef = GetNextElement(FECDef);
  }

  std::sort(fFEC.begin(), fFEC.end());

}


void TRestRawDAQMetadata::DumpFEC(const FECMetadata &fec){

    metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
    metadata << "FEC id:"<<fec.id << endl;
    metadata << "IP: "<<fec.ip[0]<<"."<<fec.ip[1]<<"."<<fec.ip[2]<<"."<<fec.ip[3]<< endl;
    metadata << "Chip type: "<<fec.chipType<<endl;
    metadata << "Clock Div: 0x"<<std::hex << fec.clockDiv <<std::dec<<endl;
      for(int i=0;i<4;i++){
        if(!fec.asic[i].isActive)continue;
        metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;
        metadata <<"ASIC "<<i<<endl;
        metadata << "Polarity: "<<fec.asic[i].polarity<<endl;
        metadata <<"Gain: 0x"<<std::hex << fec.asic[i].gain <<std::dec<<endl;
        metadata <<"ShappingTime: 0x"<<std::hex << fec.asic[i].shappingTime <<std::dec<<endl;
        metadata <<"Channel start: "<< fec.asic[i].channelStart <<endl;
        metadata <<"Channel end: "<< fec.asic[i].channelEnd <<endl;
        metadata <<"Active channels: "<<endl;
          for(int c=0;c<79;c++){
            if(fec.asic[i].channelActive[c])metadata<<c<<"; ";
            if(c>0 && c%10 == 0)metadata<<endl;
          }
        metadata<<endl;
      }
    metadata << "+++++++++++++++++++++++++++++++++++++++++++++" << endl;

}

