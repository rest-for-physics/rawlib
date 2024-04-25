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

#ifndef RESTProc_TRestRawBaseLineCorrectionProcess
#define RESTProc_TRestRawBaseLineCorrectionProcess

#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"
#include "TRestRawReadoutMetadata.h"

class TRestRawBaseLineCorrectionProcess : public TRestEventProcess {
   private:
    // We define specific input/output event data holders
    TRestRawSignalEvent* fInputEvent;   //!        
    TRestRawSignalEvent* fOutputEvent;  //!
	TRestRawReadoutMetadata* fReadoutMetadata = nullptr; 

    void Initialize() override;

    /// It defines the signals id range where analysis is applied
	TVector2 fSignalsRange = {-1, -1};

    /// Time window width in bins for the moving average filter for baseline correction
	UShort_t fSmoothingWindow = 75;

    /// Just a flag to quickly determine if we have to apply the range filter
    Bool_t fRangeEnabled = false;  //!

	/// Specify the channel types we want the process to be applied for
	std::set<std::string> fChannelTypes = {};

   public:
    RESTValue GetInputEvent() const override { return fInputEvent; }
    RESTValue GetOutputEvent() const override { return fOutputEvent; }

    void InitProcess() override;

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

	void InitFromConfigFile() override;

	void PrintMetadata() override {
		BeginPrintProcess();

		if (!fChannelTypes.empty()) {
		    RESTMetadata << "Selected channel types: ";
		    // Iterate through each channel type in fChannelTypes and print it
		    for (auto it = fChannelTypes.begin(); it != fChannelTypes.end(); ++it) {
		        RESTMetadata << *it;
		        // If it's not the last element, print a comma and a space
		        if (std::next(it) != fChannelTypes.end()) {
		            RESTMetadata << ", ";
		        }
		    }
		    RESTMetadata << RESTendl;
		} else {
		    RESTMetadata << "No channel type chosen." << RESTendl;
		}
		
		RESTMetadata << "Smoothing window size: " << fSmoothingWindow << RESTendl;
		RESTMetadata << "Baseline correction applied to signals with IDs in range (" << fSignalsRange.X()
		             << "," << fSignalsRange.Y() << ")" << RESTendl;

		EndPrintProcess();
	}
    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawBaseLineCorrectionProcess; }

    /// Returns the name of this process
    const char* GetProcessName() const override { return "baseLineCorrection"; }

    TRestRawBaseLineCorrectionProcess();
    ~TRestRawBaseLineCorrectionProcess();

    // ROOT class definition helper. Increase the number in it every time
    // you add/rename/remove the process parameters
    ClassDefOverride(TRestRawBaseLineCorrectionProcess, 1);
};
#endif
