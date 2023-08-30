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

#ifndef RestCore_TRestRawFindResponseSignalProcess
#define RestCore_TRestRawFindResponseSignalProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"

//! A process to find a representative signal to generate a response signal
class TRestRawFindResponseSignalProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fInputSignalEvent;   //!
    TRestRawSignalEvent* fOutputSignalEvent;  //!

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fInputSignalEvent; }
    RESTValue GetOutputEvent() const override { return fOutputSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        EndPrintProcess();
    }

    TRestMetadata* GetProcessMetadata() const { return nullptr; }

    const char* GetProcessName() const override { return "findResponseSignal"; }

    TRestRawFindResponseSignalProcess();
    TRestRawFindResponseSignalProcess(const char* configFilename);
    ~TRestRawFindResponseSignalProcess();

    ClassDefOverride(TRestRawFindResponseSignalProcess, 1);
};
#endif
