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

#ifndef RestCore_TRestRawSignalAnalysisProcess
#define RestCore_TRestRawSignalAnalysisProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"

//! An analysis process to extract valuable information from a TRestRawSignalEvent.
class TRestRawSignalAnalysisProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fSignalEvent;  //!

    /// Just a flag to quickly determine if we have to apply the range filter
    Bool_t fRangeEnabled = false;  //!

    /// The range where the baseline range will be calculated
    TVector2 fBaseLineRange = TVector2(5, 55);

    /// The range where the observables will be calculated
    TVector2 fIntegralRange = TVector2(10, 500);

    /// Option for calculation of baseline parameters, can be set to "ROBUST"
    std::string fBaseLineOption;  //<

    /// The number of sigmas over baseline fluctuations to identify a point over threshold
    Double_t fPointThreshold = 2;

    /// A parameter to define a minimum signal fluctuation. Measured in sigmas.
    Double_t fSignalThreshold = 5;

    /// The minimum number of points over threshold to identify a signal as such
    Int_t fPointsOverThreshold = 5;

    /// It defines the signals id range where analysis is applied
    TVector2 fSignalsRange = TVector2(-1, -1);  //<

    TRestRawReadoutMetadata* fReadoutMetadata = nullptr;

    void Initialize() override;

    std::set<std::string> fChannelTypes = {};  // this process will only be applied to selected channel types

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fSignalEvent; }
    RESTValue GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    static TRestRawReadoutMetadata* Metadata;

    void InitFromConfigFile() override;

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Baseline range : ( " << fBaseLineRange.X() << " , " << fBaseLineRange.Y() << " ) "
                     << RESTendl;
        RESTMetadata << "Integral range : ( " << fIntegralRange.X() << " , " << fIntegralRange.Y() << " ) "
                     << RESTendl;
        RESTMetadata << "Point Threshold : " << fPointThreshold << " sigmas" << RESTendl;
        RESTMetadata << "Signal threshold : " << fSignalThreshold << " sigmas" << RESTendl;
        RESTMetadata << "Number of points over threshold : " << fPointsOverThreshold << RESTendl;
        RESTMetadata << "Baseline calculation method: " << (fBaseLineOption.empty() ? "Standard" : "Robust")
                     << RESTendl;

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "rawSignalAnalysis"; }

    TRestRawSignalAnalysisProcess();
    ~TRestRawSignalAnalysisProcess();

    ClassDefOverride(TRestRawSignalAnalysisProcess, 5);
};
#endif
