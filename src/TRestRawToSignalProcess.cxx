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
/// \warning **⚠ REST is under continuous development.** This
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

TRestRawToSignalProcess::TRestRawToSignalProcess() { Initialize(); }

TRestRawToSignalProcess::TRestRawToSignalProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

TRestRawToSignalProcess::~TRestRawToSignalProcess() {
    // TRestRawToSignalProcess destructor
    delete fSignalEvent;
}

void TRestRawToSignalProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name) == -1) {
        cout << "Loading default" << endl;
        LoadDefaultConfig();
    }
}

void TRestRawToSignalProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    delete fSignalEvent;
    fSignalEvent = new TRestRawSignalEvent();

    fInputBinFile = nullptr;

    fMinPoints = 512;

    fSingleThreadOnly = true;
    fIsExternal = true;
    fgKeepFileOpen = true;

    totalBytes = 0;
    totalbytesRead = 0;
}

void TRestRawToSignalProcess::InitFromConfigFile() {
    fElectronicsType = GetParameter("electronics");
    fShowSamples = StringToInteger(GetParameter("showSamples", "10"));
    fMinPoints = StringToInteger(GetParameter("minPoints", "512"));

    PrintMetadata();

    if (fElectronicsType == "SingleFeminos" || fElectronicsType == "TCMFeminos" || fElectronicsType == "TDS")
        return;

    if (fElectronicsType == "FEUDream") {
        fgKeepFileOpen = false;
        return;
    }

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Warning) {
        cout << "REST WARNING: TRestRawToSignalProcess::InitFromConfigFile" << endl;
        cout << "Electronic type " << fElectronicsType << " not found " << endl;
        // cout << "Loading default config" << endl;
    }

    LoadDefaultConfig();
}

void TRestRawToSignalProcess::LoadDefaultConfig() {
    fElectronicsType = "SingleFeminos";
    fMinPoints = 512;
}

Bool_t TRestRawToSignalProcess::OpenInputFiles(const vector<string>& files) {
    nFiles = 0;
    fInputFiles.clear();
    fInputFileNames.clear();
    totalBytes = 0;
    totalbytesRead = 0;

    for (const auto& file : files) {
        AddInputFile(file);
    }

    if (nFiles > 0) {
        fInputBinFile = fInputFiles[0];
    } else {
        RESTError << "No input file is opened, in process: " << this->ClassName() << "!" << RESTendl;
        exit(1);
    }

    RESTDebug << this->GetName() << " : opened " << nFiles << " files" << RESTendl;
    return nFiles;
}

Bool_t TRestRawToSignalProcess::AddInputFile(const string& file) {
    for (auto& inputFileName : fInputFileNames) {
        if (inputFileName == file) {
            RESTError << "file: \"" << file << "\" already added!" << RESTendl;
            return false;
        }
    }

    FILE* f = fopen(file.c_str(), "rb");

    if (f == nullptr) {
        RESTWarning << "REST WARNING. Input file for " << this->ClassName() << " does not exist!" << RESTendl;
        RESTWarning << "File : " << file << RESTendl;
        return false;
    }

    fInputFiles.push_back(f);
    fInputFileNames.push_back(file);

    struct stat statbuf;
    stat(file.c_str(), &statbuf);
    totalBytes += statbuf.st_size;

    nFiles++;

    return true;
}

Bool_t TRestRawToSignalProcess::ResetEntry() {
    for (auto f : fInputFiles) {
        if (f != nullptr) {
            if (fseek(f, 0, 0) != 0) return false;
        }
    }
    InitProcess();

    return true;
}

void TRestRawToSignalProcess::PrintMetadata() {
    BeginPrintProcess();

    RESTMetadata << " " << RESTendl;
    RESTMetadata << " ==================================== " << RESTendl;
    RESTMetadata << "DAQ : " << GetTitle() << RESTendl;
    RESTMetadata << "Electronics type : " << fElectronicsType << RESTendl;
    RESTMetadata << "Minimum number of points : " << fMinPoints << RESTendl;
    RESTMetadata << "All raw files open at beginning : " << fgKeepFileOpen << RESTendl;
    RESTMetadata << " ==================================== " << RESTendl;

    RESTMetadata << " " << RESTendl;

    EndPrintProcess();
}

Bool_t TRestRawToSignalProcess::GoToNextFile() {
    iCurFile++;
    if (iCurFile < nFiles) {
        if (fgKeepFileOpen) {
            fInputBinFile = fInputFiles[iCurFile];
        } else {
            fclose(fInputBinFile);
            fInputBinFile = fopen(fInputFileNames[iCurFile].c_str(), "rb");
        }
        RESTInfo << "GoToNextFile(): Going to the next raw input file number " << iCurFile << " over "
                 << nFiles << RESTendl;
        RESTInfo << "                Reading file name:  " << fInputFileNames[iCurFile] << RESTendl;
        return true;
    } else {
        RESTInfo << "GoToNextFile(): No more file to read" << RESTendl;
    }
    return false;
}
