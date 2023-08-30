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

#ifndef RestCore_TRestRawSignalAddNoiseProcess
#define RestCore_TRestRawSignalAddNoiseProcess

#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"

//! A process to add/emulate electronic noise into a TRestRawSignalEvent
class TRestRawSignalAddNoiseProcess : public TRestEventProcess {
   private:
    /// A pointer to the input signal event
    TRestRawSignalEvent* fInputSignalEvent;  //!

    /// A pointer to the output signal event
    TRestRawSignalEvent* fOutputSignalEvent;  //!

    void Initialize() override;

    void LoadDefaultConfig();

    /// The noise level to be added to the signal. It is 1-gaussian sigma
    Double_t fNoiseLevel = 10.0;

   public:
    /// It returns the noise level defined in the process (ADC units)
    inline Double_t GetNoiseLevel() const { return fNoiseLevel; }

    /// It sets the noise level of the process (ADC units)
    inline void SetNoiseLevel(Double_t noiseLevel) { fNoiseLevel = noiseLevel; }

    /// Returns a pointer to the input signal event
    RESTValue GetInputEvent() const override { return fInputSignalEvent; }

    /// Returns a pointer to the output signal event
    RESTValue GetOutputEvent() const override { return fOutputSignalEvent; }

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    /// Prints out the metadata members of this class
    inline void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Noise Level : " << fNoiseLevel << RESTendl;

        EndPrintProcess();
    }

    TRestMetadata* GetProcessMetadata() const { return nullptr; }

    /// Returns a given process name
    const char* GetProcessName() const override { return "rawSignalAddNoise"; }

    TRestRawSignalAddNoiseProcess();
    TRestRawSignalAddNoiseProcess(const char* configFilename);
    ~TRestRawSignalAddNoiseProcess();

    ClassDefOverride(TRestRawSignalAddNoiseProcess, 2);
};
#endif
