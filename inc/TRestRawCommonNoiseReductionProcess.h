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

#ifndef RestCore_TRestRawCommonNoiseReductionProcess
#define RestCore_TRestRawCommonNoiseReductionProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"
#include "TRestRawSignal.h"

//! A process to subtract the common channels noise from RawSignal type
class TRestRawCommonNoiseReductionProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fInputEvent;

    /// A pointer to the specific TRestRawSignalEvent output
    TRestRawSignalEvent* fOutputEvent;

    /// Common noise to all signals or by groups (It can be 0 or 1).
    Int_t fBlocks = 0;

    /// The mode defines the method to be used (It can be 0 or 1).
    Int_t fMode = 0;

    /// The percentage of signals taken from the array center to be considered for
    /// the average.
    Int_t fCenterWidth = 10;

    /// Minimum number of signals required to apply the process.
    Int_t fMinSignalsRequired = 200;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fInputEvent; }
    RESTValue GetOutputEvent() const override { return fOutputEvent; }

    void InitProcess() override;

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << " mode : [" << fMode << "]";
        if (fMode == 0) RESTMetadata << " --> Mode 0 activated." << RESTendl;
        if (fMode == 1) RESTMetadata << " --> Mode 1 activated." << RESTendl;
        RESTMetadata << " centerWidth : " << fCenterWidth << RESTendl;
        RESTMetadata << "blocks : [" << fBlocks << "]" << RESTendl;
        RESTMetadata << " Minimum number of signals : " << fMinSignalsRequired << RESTendl;

        EndPrintProcess();
    }

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawCommonNoiseReductionProcess; }

    /// Returns the reduced process name
    const char* GetProcessName() const override { return "commonNoiseReduction"; }

    // Constructor
    TRestRawCommonNoiseReductionProcess();
    TRestRawCommonNoiseReductionProcess(const char* configFilename);

    // Destructor
    ~TRestRawCommonNoiseReductionProcess();

    ClassDefOverride(TRestRawCommonNoiseReductionProcess, 2);
};
#endif
