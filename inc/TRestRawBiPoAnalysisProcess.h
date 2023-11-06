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

#ifndef RESTProc_TRestRawBiPoAnalysisProcess
#define RESTProc_TRestRawBiPoAnalysisProcess

#include "TRestEventProcess.h"
#include "TRestRawSignalEvent.h"

class TRestRawBiPoAnalysisProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input event
    TRestRawSignalEvent* fAnaEvent;  //!

    void Initialize() override;

   public:
    RESTValue GetInputEvent() const override { return fAnaEvent; }
    RESTValue GetOutputEvent() const override { return fAnaEvent; }

    void InitProcess() override;

    const char* GetProcessName() const override { return "BiPoAnalysis"; }

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

    void PrintMetadata() override {
        BeginPrintProcess();

        EndPrintProcess();
    }

    TRestRawBiPoAnalysisProcess();
    ~TRestRawBiPoAnalysisProcess();

    ClassDefOverride(TRestRawBiPoAnalysisProcess, 1);
};
#endif
