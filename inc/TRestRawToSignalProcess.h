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

#ifndef RestCore_TRestRawToSignalProcess
#define RestCore_TRestRawToSignalProcess

#include <TRestEventProcess.h>
#include <TRestRawSignalEvent.h>

//! A base class for any process reading a binary external file as input to REST
class TRestRawToSignalProcess : public TRestEventProcess {
   protected:
    virtual void InitFromConfigFile();
    unsigned int payload;
    unsigned int frameBits;
    std::string fElectronicsType;  // AFTER or AGET
    Int_t fMinPoints;

    Double_t tStart;
    Long64_t totalBytesReaded;
    Long64_t totalBytes;

    TRestRawSignalEvent* fSignalEvent = 0;  //!
#ifndef __CINT__
    FILE* fInputBinFile;  //!

    Int_t fRunOrigin;     //!
    Int_t fSubRunOrigin;  //!

    Int_t nFiles;                    //!
    Int_t iCurFile;                  //!
    std::vector<FILE*> fInputFiles;  //!
    std::vector<std::string> fInputFileNames;
    bool fgKeepFileOpen;  //! true if need to open all raw files at the beginning

    Int_t fShowSamples;  //!
#endif

    void LoadDefaultConfig();

   public:
    any GetInputEvent() const override { return any((TRestEvent*)nullptr); }
    any GetOutputEvent() const override { return fSignalEvent; }

    virtual void Initialize();
    virtual void InitProcess() = 0;
    virtual TRestEvent* ProcessEvent(TRestEvent* evInput) = 0;
    virtual void EndProcess();
    // virtual TString GetProcessName()=0;
    TRestMetadata* GetProcessMetadata() const { return nullptr; }

    void SetRunOrigin(Int_t runOrigin) { fRunOrigin = runOrigin; }
    void SetSubRunOrigin(Int_t subRunOrigin) { fSubRunOrigin = subRunOrigin; }

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    virtual void PrintMetadata();

    // Bool_t OpenInputBinFile(TString fName);

    virtual Bool_t OpenInputFiles(std::vector<std::string> files);
    virtual Bool_t AddInputFile(std::string file);
    Bool_t ResetEntry();

    virtual Long64_t GetTotalBytesRead() const { return totalBytesReaded; }
    virtual Long64_t GetTotalBytes() const { return totalBytes; }
    //  Int_t GetRunNumber(){return fRunNumber;}
    //  Int_t GetRunIndex(){return fRunIndex;}
    virtual std::string GetElectronicsType() const { return fElectronicsType; }

    Bool_t GoToNextFile();

    // Constructor
    TRestRawToSignalProcess();
    TRestRawToSignalProcess(const char* configFilename);
    // Destructor
    ~TRestRawToSignalProcess();

    ClassDef(TRestRawToSignalProcess, 1);
};
#endif
