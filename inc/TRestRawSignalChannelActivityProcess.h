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

#ifndef RestCore_TRestRawSignalChannelActivityProcess
#define RestCore_TRestRawSignalChannelActivityProcess

#include <TH1D.h>
#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"

//! A pure analysis process to generate histograms with detector channels
//! activity
class TRestRawSignalChannelActivityProcess : public TRestEventProcess {
   protected:
    /// The value of the lower signal threshold to add it to the histogram
    Double_t fLowThreshold = 25;

    /// The value of the higher signal threshold to add it to the histogram
    Double_t fHighThreshold = 50;

    /// The number of bins at the daq channels histogram
    Int_t fDaqChannels = 300;

    /// The first channel at the daq channels histogram
    Int_t fDaqStartChannel = 4320;

    /// The last channel at the daq channels histogram
    Int_t fDaqEndChannel = 4620;

    /// The daq channels histogram
    TH1D* fDaqChannelsHisto = nullptr;  //!

   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fInputEvent = nullptr;  //!

    std::string fChannelType;
    TRestRawReadoutMetadata* fReadoutMetadata = nullptr;  //!

    void Initialize() override;

   public:
    RESTValue GetInputEvent() const override { return fInputEvent; }
    RESTValue GetOutputEvent() const override { return fInputEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    /// It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override {
        BeginPrintProcess();

        if (!fChannelType.empty()) {
            RESTMetadata << "channelType : " << fChannelType << RESTendl;
        }

        RESTMetadata << "Low signal threshold activity : " << fLowThreshold << RESTendl;
        RESTMetadata << "High signal threshold activity : " << fHighThreshold << RESTendl;

        RESTMetadata << "Number of daq histogram channels : " << fDaqChannels << RESTendl;
        RESTMetadata << "Start daq channel : " << fDaqStartChannel << RESTendl;
        RESTMetadata << "End daq channel : " << fDaqEndChannel << RESTendl;

        EndPrintProcess();
    }

    /// Returns the name of this process
    const char* GetProcessName() const override { return "rawSignalChannelActivity"; }

    TRestRawSignalChannelActivityProcess();
    ~TRestRawSignalChannelActivityProcess();

    ClassDefOverride(TRestRawSignalChannelActivityProcess, 5);
};
#endif
