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

#ifndef RestCore_TRestRawFeminosRootToSignalProcess
#define RestCore_TRestRawFeminosRootToSignalProcess

#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"
#include "TRestRawToSignalProcess.h"

///
/// Read data from the root file output of https://github.com/rest-for-physics/feminos-daq into a
/// TRestRawSignalEvent
///

class TRestRawFeminosRootToSignalProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fSignalEvent = nullptr;  //!
    Long64_t fInputTreeEntry = 0;                 //!

    TFile* fInputFile = nullptr;       //!
    TTree* fInputEventTree = nullptr;  //!
    TTree* fInputRunTree = nullptr;    //!

    ULong64_t fInputEventTreeTimestamp = 0;                              //!
    std::vector<unsigned short>* fInputEventTreeSignalIds = nullptr;     //!
    std::vector<unsigned short>* fInputEventTreeSignalValues = nullptr;  //!

   public:
    RESTValue GetInputEvent() const override { return RESTValue((TRestEvent*)nullptr); }
    RESTValue GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    void Initialize() override;

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    const char* GetProcessName() const override { return "FeminosRootToSignal"; }

    // Constructor
    TRestRawFeminosRootToSignalProcess();
    TRestRawFeminosRootToSignalProcess(const char* configFilename);

    // Destructor
    ~TRestRawFeminosRootToSignalProcess();

    ClassDefOverride(TRestRawFeminosRootToSignalProcess,
                     1);  // Template for a REST "event process" class inherited from
                          // TRestEventProcess
};
#endif
