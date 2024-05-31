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

#ifndef RESTProc_TRestRawZeroSupressionToRawProcess
#define RESTProc_TRestRawZeroSupressionToRawProcess

#include "TRestEvent.h"
#include "TRestEventProcess.h"
#include "TRestRawSignalEvent.h"

/// This process remove the offset on a zerosuppression acquired event
class TRestRawZeroSupressionToRawProcess : public TRestEventProcess {
   private:
    /// Pointer to TRestRawSignalEvent input event
    TRestRawSignalEvent* fEvent;  //!

    void Initialize() override;

   public:
    RESTValue GetInputEvent() const override { return fEvent; }
    RESTValue GetOutputEvent() const override { return fEvent; }

    void InitProcess() override;

    const char* GetProcessName() const override { return "ZeroSupressionToRaw"; }

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

    /// It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override {
        BeginPrintProcess();

        EndPrintProcess();
    }

    TRestRawZeroSupressionToRawProcess();
    ~TRestRawZeroSupressionToRawProcess();

    ClassDefOverride(TRestRawZeroSupressionToRawProcess, 1);
};
#endif
