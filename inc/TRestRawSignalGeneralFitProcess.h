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

#ifndef RestCore_TRestRawSignalGeneralFitProcess
#define RestCore_TRestRawSignalGeneralFitProcess

#include <TRestRawSignalEvent.h>

#include "TF1.h"
#include "TH1D.h"
#include "TRestEventProcess.h"

//! An analysis REST process to extract valuable information from RawSignal type
//! of data.
class TRestRawSignalGeneralFitProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fRawSignalEvent;  //!

    void Initialize();

    void LoadDefaultConfig();

    TVector2 fFunctionRange = TVector2(0, 0);
    std::string fFunction;

    TF1* fFitFunc = nullptr;  //!

    /*Double_t fShaping = 0;
    Double_t fStartPosition = 0;
    Double_t fBaseline = 0;
    Double_t fAmplitude = 0;*/

   protected:
    // add here the members of your event process

   public:
    inline any GetInputEvent() const { return fRawSignalEvent; }
    inline any GetOutputEvent() const { return fRawSignalEvent; }

    TF1* GetFunction() { return fFitFunc; }

    /*Double_t GetShaping() { return fShaping; }
   inline Double_t GetStartPosition() const { return fStartPosition; }
   inline Double_t GetBaseline() const { return fBaseline; }
   inline Double_t GetAmplitude() const { return fAmplitude; }*/

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string configFilename, std::string name = "");

    void PrintMetadata() {
        BeginPrintProcess();

        metadata << "Function std::string: " << fFunction << endl;
        metadata << "Range: ( " << fFunctionRange.X() << " , " << fFunctionRange.Y() << " ) " << endl;

        EndPrintProcess();
    }

    inline const char* GetProcessName() const { return "rawSignalGeneralFit"; }

    TRestRawSignalGeneralFitProcess();  // Constructor
    TRestRawSignalGeneralFitProcess(char* configFilename);
    ~TRestRawSignalGeneralFitProcess();  // Destructor

    ClassDef(TRestRawSignalGeneralFitProcess, 2);
};
#endif
