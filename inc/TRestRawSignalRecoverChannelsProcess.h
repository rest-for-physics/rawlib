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

#ifndef RestCore_TRestRawSignalRecoverChannelsProcess
#define RestCore_TRestRawSignalRecoverChannelsProcess

#ifdef REST_DetectorLib
#include <TRestDetectorReadout.h>
#endif
#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"

//! A process allowing to recover selected channels from a TRestRawSignalEvent
class TRestRawSignalRecoverChannelsProcess : public TRestEventProcess {
   private:
    std::vector<Int_t> fChannelIds;  //<

    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fInputSignalEvent;  //!

    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fOutputSignalEvent;  //!

#ifdef REST_DetectorLib
    /// A pointer to the readout previously defined inside REST.
    TRestDetectorReadout* fReadout;  //!
#endif

    void Initialize() override;

    void LoadDefaultConfig();

    void GetAdjacentSignalIds(Int_t signalId, Int_t& idLeft, Int_t& idRight, Int_t& idLeft2, Int_t& idRight2);

   public:
    any GetInputEvent() const override { return fInputSignalEvent; }
    any GetOutputEvent() const override { return fOutputSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    /// It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override {
        BeginPrintProcess();
        for (const auto& channelId : fChannelIds) {
            RESTMetadata << "Channel id to recover: " << channelId << RESTendl;
        }
        EndPrintProcess();
    }

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawSignalRecoverChannelsProcess; }

    /// Returns the name of this process
    const char* GetProcessName() const override { return "recoverChannels"; }

    // Constructor
    TRestRawSignalRecoverChannelsProcess();
    TRestRawSignalRecoverChannelsProcess(const char* configFilename);

    // Destructor
    ~TRestRawSignalRecoverChannelsProcess();

    ClassDefOverride(TRestRawSignalRecoverChannelsProcess, 1);
};
#endif
