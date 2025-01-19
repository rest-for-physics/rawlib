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

#ifndef RESTProc_TRestRawSignalRecoverSaturationProcess
#define RESTProc_TRestRawSignalRecoverSaturationProcess

#include "TRestEventProcess.h"
#include "TRestRawSignalEvent.h"

/// TODO Write here a brief description. Just one line!
class TRestRawSignalRecoverSaturationProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input event
    TRestRawSignalEvent* fAnaEvent;  //!

    void Initialize() override;

    // Add here the members or parameters for your event process.
    // You can set their default values here together.
    // Note: add "//!" mark at the end of the member definition
    // if you don't want to save them to disk.
    Size_t fMinSaturatedBins;  //<  ///< Minimum number of saturated bins to consider a signal as saturated

    Bool_t fProcessAllSignals;    //<  ///< Process all signals in the event
    Size_t fNBinsIfNotSaturated;  //<  ///< Number of bins to consider if the signal is not saturated
    Short_t fMinSaturationValue;  //<  ///< Threshold to consider a bin as saturated

    TVector2
        fBaseLineRange;  //<  ///< Range of bins to calculate the baseline and fix that parameter in the fit
    TVector2 fFitRange;  //<  ///< Range of bins to fit the signal
    TCanvas* fC;         //!  ///< Canvas to draw the signals

   public:
    RESTValue GetInputEvent() const override { return fAnaEvent; }
    RESTValue GetOutputEvent() const override { return fAnaEvent; }

    void InitProcess() override;

    const char* GetProcessName() const override { return "RawSignalRecoverSaturationProcess"; }

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

    ///  It prints out the process parameters stored in the metadata structure
    void PrintMetadata() override {
        BeginPrintProcess();

        // Write here how to print the added process members and parameters.
        std::string strProcessAllSignals = fProcessAllSignals ? "true" : "false";
        RESTMetadata << "MinSaturatedBins: " << fMinSaturatedBins << RESTendl;
        RESTMetadata << "ProcessAllSignals: " << strProcessAllSignals << RESTendl;
        RESTMetadata << "NBinsIfNotSaturated: " << fNBinsIfNotSaturated << RESTendl;
        RESTMetadata << "MinSaturationValue: " << fMinSaturationValue << RESTendl;
        RESTMetadata << "BaseLineRange: (" << fBaseLineRange.X() << ", " << fBaseLineRange.Y() << ")"
                     << RESTendl;
        RESTMetadata << "FitRange: (" << fFitRange.X() << ", " << fFitRange.Y() << ")" << RESTendl;

        EndPrintProcess();
    }

    TRestRawSignalRecoverSaturationProcess();
    ~TRestRawSignalRecoverSaturationProcess();

    // ROOT class definition helper. Increase the number in it every time
    // you add/rename/remove the process parameters
    ClassDefOverride(TRestRawSignalRecoverSaturationProcess, 1);
};
#endif
