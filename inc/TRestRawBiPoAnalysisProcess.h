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

<<<<<<< HEAD
    /// TODO Write here a brief description. Just one line!
    class TRestRawBiPoAnalysisProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input event
    TRestRawSignalEvent* fAnaEvent;  //!

    void Initialize() override;

    // Add here the members or parameters for your event process.
    // You can set their default values here together.
    // Note: add "//!" mark at the end of the member definition
    // if you don't want to save them to disk.

    /// REMOVE THIS MEMBER! A dummy member that will be written to the ROOT file.
    Double_t fDummy = 3.14;  //<

    /// REMOVE THIS MEMBER! A dummy member that will be NOT written to the ROOT file.
    Double_t fDummyVar = 3.14;  //!

   public:
    any GetInputEvent() const override { return fAnaEvent; }
    any GetOutputEvent() const override { return fAnaEvent; }

    void InitProcess() override;

    const char* GetProcessName() const override { return "BiPoAnalysis"; }

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

    ///  It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override {
        BeginPrintProcess();

        // Write here how to print the added process members and parameters.

        EndPrintProcess();
    }

    TRestRawBiPoAnalysisProcess();
    ~TRestRawBiPoAnalysisProcess();

    // ROOT class definition helper. Increase the number in it every time
    // you add/rename/remove the process parameters
    ClassDefOverride(TRestRawBiPoAnalysisProcess, 1);
};
=======
class TRestRawSignalAnalysisProcess : public TRestEventProcess {}

>>>>>>> be55b092b46a7aa84ff6a83d22ec1516bf2f470e
#endif
