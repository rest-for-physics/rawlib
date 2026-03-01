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

#ifndef RestCore_TRestRawFemDAQToSignalProcess
#define RestCore_TRestRawFemDAQToSignalProcess

#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"
#include "TRestRawToSignalProcess.h"

///
/// Read data from the root file output of femdaq into a
/// TRestRawSignalEvent
///

class TRestRawFemDAQToSignalProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fSignalEvent = nullptr;  //!
    Long64_t fInputTreeEntry = 0;                 //!

    TFile* fInputFile = nullptr;  //!
    TTree* fInputTree = nullptr;  //!

    Double_t fTimestamp = 0;                      //!
    Int_t fEventID = 0;                           //!
    std::vector<int>* fSignalIds = nullptr;       //!
    std::vector<short>* fSignalValues = nullptr;  //!
    Double_t fStartTimestamp = -1;                //!
    Double_t fEndTimestamp = 0;                   //!
    Bool_t fUseFeminosDaqRunInfo = true;          //<

   public:
    RESTValue GetInputEvent() const override { return RESTValue((TRestEvent*)nullptr); }
    RESTValue GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    void Initialize() override;
    void EndProcess() override;

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    const char* GetProcessName() const override { return "FeminosRootToSignal"; }

    /// It prints out the process parameters stored in the metadata structure
    inline void PrintMetadata() override {
        BeginPrintProcess();
        std::string useFemDaqRunInfoStr = fUseFeminosDaqRunInfo ? "true" : "false";
        RESTMetadata << "Use fem-daq run information: " << useFemDaqRunInfoStr << RESTendl;

        EndPrintProcess();
    }
    // Constructor
    TRestRawFemDAQToSignalProcess();
    TRestRawFemDAQToSignalProcess(const char* configFilename);

    // Destructor
    ~TRestRawFemDAQToSignalProcess();

    ClassDefOverride(TRestRawFemDAQToSignalProcess,
                     0);  // Template for a REST "event process" class inherited from
                          // TRestEventProcess
};
#endif
