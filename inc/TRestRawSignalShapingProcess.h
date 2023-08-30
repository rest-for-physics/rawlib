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

#ifndef RestCore_TRestRawSignalShapingProcess
#define RestCore_TRestRawSignalShapingProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"

//! A process to convolute the input raw signal event with a given input
//! response.
class TRestRawSignalShapingProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fInputSignalEvent;

    /// A pointer to the specific TRestRawSignalEvent output
    TRestRawSignalEvent* fOutputSignalEvent;

    void Initialize() override;

   protected:
    // add here the members of your event process
    TString fResponseFilename;

    /// Types are : gaus, shaper, shaperSin, responseFile
    TString fShapingType = "shaperSin";
    /// The characteristic time of the shaping
    Double_t fShapingTime = 10.0;  // ns
    /// A value used to scale the input signal
    Double_t fShapingGain = 1.0;

   public:
    inline TString GetShapingType() const { return fShapingType; }
    inline void SetShapingType(const TString& samplingType) { fShapingType = samplingType; }

    inline Double_t GetShapingTime() const { return fShapingTime; }
    inline void SetShapingTime(Double_t shapingTime) { fShapingTime = shapingTime; }

    inline Double_t GetShapingGain() const { return fShapingGain; }
    inline void SetShapingGain(Double_t shapingGain) { fShapingGain = shapingGain; }

    RESTValue GetInputEvent() const override { return fInputSignalEvent; }
    RESTValue GetOutputEvent() const override { return fOutputSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    /// It prints out the process parameters stored in the metadata structure
    inline void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Shaping type : " << fShapingType << RESTendl;
        RESTMetadata << "Shaping time : " << fShapingTime << RESTendl;
        RESTMetadata << "Amplitude gain : " << fShapingGain << RESTendl;
        if (fShapingType == "responseFile") {
            RESTMetadata << "Response file : " << fResponseFilename << RESTendl;
        }

        EndPrintProcess();
    }

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawSignalShapingProcess; }

    /// Returns the name of this process
    const char* GetProcessName() const override { return "rawSignalShaping"; }

    TRestRawSignalShapingProcess();
    TRestRawSignalShapingProcess(const char* configFilename);
    ~TRestRawSignalShapingProcess();

    ClassDefOverride(TRestRawSignalShapingProcess, 2);
};
#endif
