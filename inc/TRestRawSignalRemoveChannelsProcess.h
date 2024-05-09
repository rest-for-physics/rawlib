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

#ifndef RestCore_TRestRawSignalRemoveChannelsProcess
#define RestCore_TRestRawSignalRemoveChannelsProcess

#include <TRestEventProcess.h>

#include "TRestRawReadoutMetadata.h"
#include "TRestRawSignalEvent.h"

//! A process allowing to remove selected channels from a TRestRawSignalEvent
class TRestRawSignalRemoveChannelsProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestDetectorSignalEvent input
    TRestRawSignalEvent* fInputEvent;  //!

    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fOutputEvent;  //!

    /// A pointer to the readout metadata
    TRestRawReadoutMetadata* fReadoutMetadata = nullptr;  //!

    void InitFromConfigFile() override;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    std::vector<Int_t> fChannelIds;
    std::vector<std::string> fChannelTypes;

    std::map<Int_t, std::string> fChannelTypesToRemove;

   public:
    RESTValue GetInputEvent() const override { return fInputEvent; }
    RESTValue GetOutputEvent() const override { return fOutputEvent; }

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    std::vector<std::string> GetChannelTypes() const { return fChannelTypes; }
    std::vector<Int_t> GetChannelIds() const { return fChannelIds; }

    /// It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override;

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawSignalRemoveChannelsProcess; }

    /// Returns the name of this process
    const char* GetProcessName() const override { return "removeChannels"; }

    // Constructor
    TRestRawSignalRemoveChannelsProcess();
    TRestRawSignalRemoveChannelsProcess(const char* configFilename);

    // Destructor
    ~TRestRawSignalRemoveChannelsProcess();

    ClassDefOverride(TRestRawSignalRemoveChannelsProcess, 3);
};
#endif
