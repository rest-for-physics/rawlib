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
///
/// The TRestRawToSignalProcess ... is NOT documented.
///
/// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
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
/// 2015-June: First implementation of abstract class for binary format reading
///             Juanan Garcia
///
/// \class      TRestRawToSignalProcess
/// \author     Juanan Garcia
///
/// <hr>
///
#include "TRestRawToSignalProcess.h"

#include <sys/stat.h>
using namespace std;
#include "TTimeStamp.h"

ClassImp(TRestRawToSignalProcess);
//______________________________________________________________________________
TRestRawToSignalProcess::TRestRawToSignalProcess() { Initialize(); }

TRestRawToSignalProcess::TRestRawToSignalProcess(char *cfgFileName) {
  Initialize();

  if (LoadConfigFromFile(cfgFileName))
    LoadDefaultConfig();
}

//______________________________________________________________________________
TRestRawToSignalProcess::~TRestRawToSignalProcess() {
  // TRestRawToSignalProcess destructor
  if (fSignalEvent)
    delete fSignalEvent;
}

void TRestRawToSignalProcess::LoadConfig(string cfgFilename, string name) {
  if (LoadConfigFromFile(cfgFilename, name) == -1) {
    cout << "Loading default" << endl;
    LoadDefaultConfig();
  }
}

//______________________________________________________________________________
void TRestRawToSignalProcess::Initialize() {
  SetSectionName(this->ClassName());

  if (fSignalEvent)
    delete fSignalEvent;
  fSignalEvent = new TRestRawSignalEvent();

  fInputBinFile = NULL;

  fMinPoints = 512;

  fSingleThreadOnly = true;
  fIsExternal = true;

  totalBytes = 0;
  totalBytesReaded = 0;
}

void TRestRawToSignalProcess::InitFromConfigFile() {
  fElectronicsType = GetParameter("electronics");
  fShowSamples = StringToInteger(GetParameter("showSamples", "10"));
  fMinPoints = StringToInteger(GetParameter("minPoints", "512"));

  PrintMetadata();

  if (fElectronicsType == "SingleFeminos" || fElectronicsType == "TCMFeminos")
    return;

  if (GetVerboseLevel() >= REST_Warning) {
    cout << "REST WARNING: TRestRawToSignalProcess::InitFromConfigFile" << endl;
    cout << "Electronic type " << fElectronicsType << " not found " << endl;
    // cout << "Loading default config" << endl;
  }

  LoadDefaultConfig();
}

void TRestRawToSignalProcess::LoadDefaultConfig() {
  // if (GetVerboseLevel() <= REST_Warning) {
  //    cout << "REST WARNING: TRestRawToSignalProcess " << endl;
  //    cout << "Error Loading config file " << endl;
  //}

  // if (GetVerboseLevel() >= REST_Debug) GetChar();

  fElectronicsType = "SingleFeminos";
  fMinPoints = 512;
}

//______________________________________________________________________________
void TRestRawToSignalProcess::EndProcess() {
  // close binary file??? Already done
}

Bool_t TRestRawToSignalProcess::OpenInputFiles(vector<string> files) {
  nFiles = 0;
  // for (auto a : fInputFiles) { delete a; }
  fInputFiles.clear();
  fInputFileNames.clear();
  totalBytes = 0;
  totalBytesReaded = 0;

  for (int i = 0; i < files.size(); i++) {
    FILE *f = fopen(files[i].c_str(), "rb");

    if (f == NULL) {
      warning << "REST WARNING. Input file for " << this->ClassName()
              << " does not exist!" << endl;
      warning << "File : " << files[i] << endl;
      continue;
    }

    fInputFiles.push_back(f);
    fInputFileNames.push_back(files[i]);

    struct stat statbuf;
    stat(files[i].c_str(), &statbuf);
    totalBytes += statbuf.st_size;

    nFiles++;
  }

  if (nFiles > 0) {
    fInputBinFile = fInputFiles[0];
  } else {
    ferr << "No input file is opened, in process: " << this->ClassName() << "!"
         << endl;
    exit(1);
  }

  debug << this->GetName() << " : opened " << nFiles << " files" << endl;
  return nFiles;
}

// For debugging
void TRestRawToSignalProcess::printBits(unsigned short num) {
  for (unsigned short bit = 0; bit < (sizeof(unsigned short) * 8); bit++) {
    printf("%i ", num & 0x01);
    num = num >> 1;
  }

  printf("\n");
}

// For debugging
void TRestRawToSignalProcess::printBits(unsigned int num) {
  for (unsigned int bit = 0; bit < (sizeof(unsigned int) * 8); bit++) {
    printf("%i ", num & 0x01);
    num = num >> 1;
  }

  printf("\n");
}

void TRestRawToSignalProcess::PrintMetadata() {
  BeginPrintProcess();

  metadata << " " << endl;
  metadata << " ==================================== " << endl;
  metadata << "DAQ : " << GetTitle() << endl;
  metadata << "Electronics type : " << fElectronicsType << endl;
  metadata << "Minimum number of points : " << fMinPoints << endl;
  metadata << " ==================================== " << endl;

  metadata << " " << endl;

  EndPrintProcess();
}
