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
/// \warning **⚠ WARNING: REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// [Contribution
/// Guide](https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md)
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

TRestRawDAQMetadata::TRestRawDAQMetadata(char *cfgFileName)
    : TRestMetadata(cfgFileName) {
  Initialize();

  LoadConfigFromFile(fConfigFileName);

  SetScriptsBuffer();
  SetParFromPedBuffer();
}

void TRestRawDAQMetadata::Initialize() { SetSectionName(this->ClassName()); }

//______________________________________________________________________________
TRestRawDAQMetadata::~TRestRawDAQMetadata() {
  cout << "Deleting TRestRawDAQMetadata" << endl;
}

//______________________________________________________________________________
void TRestRawDAQMetadata::InitFromConfigFile() {
  // string daqString;

  fNamePedScript = GetParameter("pedScript");
  if (fNamePedScript == "") {
    cout << "Pedestal script " << endl;
  }

  fNameRunScript = GetParameter("runScript");
  if (fNameRunScript == "") {
    cout << "Run script " << endl;
  }

  fElectronicsType = GetParameter("electronics");
  if (fElectronicsType == "") {
    cout << "electronic type not found " << endl;
  }
}

void TRestRawDAQMetadata::PrintMetadata() {
  cout << endl;
  cout << "====================================" << endl;
  cout << "DAQ : " << GetTitle() << endl;
  cout << "Pedestal script : " << fNamePedScript.Data() << endl;
  cout << "Run script : " << fNameRunScript.Data() << endl;
  cout << "Electronics type : " << fElectronicsType.Data() << endl;
  cout << "Gain : " << GetGain() << endl;
  cout << "Shapping time : " << GetShappingTime() << endl;
  cout << "====================================" << endl;

  cout << endl;
}

void TRestRawDAQMetadata::SetScriptsBuffer() {
  TString folder = REST_PATH;
  folder.Append("data/acquisition/");

  TString fName;

  fName = folder + fNamePedScript;

  ifstream file(fName);
  if (!file) {
    cout << __PRETTY_FUNCTION__ << " ERROR:FILE " << fName << " not found "
         << endl;
    return;
  }

  string line;
  while (getline(file, line)) {
    fPedBuffer.push_back(line);
  }

  file.close();

  fName = folder + fNameRunScript;

  ifstream file2(fName);
  if (!file2) {
    cout << __PRETTY_FUNCTION__ << " ERROR:FILE " << fName << " not found "
         << endl;
    return;
  }

  while (getline(file2, line)) {
    fRunBuffer.push_back(line);
  }

  file2.close();
}

void TRestRawDAQMetadata::PrintRunScript() {
  for (unsigned int i = 0; i < fRunBuffer.size(); i++)
    cout << fRunBuffer[i].Data() << endl;
}

void TRestRawDAQMetadata::PrintPedScript() {
  for (unsigned int i = 0; i < fPedBuffer.size(); i++)
    cout << fPedBuffer[i].Data() << endl;
}

void TRestRawDAQMetadata::SetParFromPedBuffer() {
  for (unsigned int i = 0; i < fPedBuffer.size(); i++) {
    if (fPedBuffer[i].Contains("aget * gain * "))
      fGain = GetValFromString("aget * gain * ", fPedBuffer[i]);

    if (fPedBuffer[i].Contains("aget * time "))
      fShappingTime = GetValFromString("aget * time ", fPedBuffer[i]);

    if (fPedBuffer[i].Contains("after * gain * "))
      fGain = GetValFromString("after * gain * ", fPedBuffer[i]);

    if (fPedBuffer[i].Contains("after * time "))
      fShappingTime = GetValFromString("after * time ", fPedBuffer[i]);
  }
}

UInt_t TRestRawDAQMetadata::GetValFromString(TString var, TString line) {
  unsigned int val;

  unsigned int varSize = var.Sizeof();
  unsigned int lineSize = line.Sizeof();

  // cout<<varSize<<"  "<<lineSize<<endl;

  TString diff(line(varSize - 1, lineSize - 1));

  cout << diff.Data() << endl;

  sscanf(diff.Data(), "0x%x", &val);

  return val;
}
