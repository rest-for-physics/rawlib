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

#ifndef RestCore_TRestRawSignalIdTaggingProcess
#define RestCore_TRestRawSignalIdTaggingProcess

#include <TRestRawSignalEvent.h>

#include "TRestEventProcess.h"

//! An analysis process helping to assign tags to user defined ranges of signal ids.
class TRestRawSignalIdTaggingProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fSignalEvent;  //!

    /// A list containing the tag names for each signal ids range
    std::vector<std::string> fTagNames;

    /// A list containing the id range for each tag
    std::vector<TVector2> fIdRanges;

    //// Parameters to identify good signals ////

    /// The range where the baseline range will be calculated
    TVector2 fBaseLineRange = TVector2(-1, -1);

    /// The number of sigmas over baseline fluctuations to identify a point overthreshold
    Double_t fPointThreshold = -1;

    /// A parameter to define a minimum signal fluctuation. Measured in sigmas.
    Double_t fSignalThreshold = -1;

    /// The minimum number of points over threshold to identify a signal as such
    Int_t fPointsOverThreshold = -1;

    /// Properly initialized GoodSignals parameters (fBaseLineRange, fPointThreshold, fSignalThreshold,
    /// fPointsOverThreshold)
    bool fGoodSignalsOnly = false;

    void Initialize() override;
    void InitFromConfigFile() override;

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fSignalEvent; }
    RESTValue GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Tag code: " << RESTendl;
        for (unsigned int n = 0; n < fIdRanges.size(); n++) {
            RESTMetadata << n + 1 << " - " << fTagNames[n] << ": ( " << fIdRanges[n].X() << ", "
                         << fIdRanges[n].Y() << " )" << RESTendl;
        }
        RESTMetadata << " " << RESTendl;
        RESTMetadata << "Only good signals: " << std::boolalpha << fGoodSignalsOnly << RESTendl;

        if (fGoodSignalsOnly == true) {
            RESTMetadata << "Baseline range : ( " << fBaseLineRange.X() << " , " << fBaseLineRange.Y()
                         << " ) " << RESTendl;
            RESTMetadata << "Point Threshold : " << fPointThreshold << " sigmas" << RESTendl;
            RESTMetadata << "Signal threshold : " << fSignalThreshold << " sigmas" << RESTendl;
            RESTMetadata << "Number of points over threshold : " << fPointsOverThreshold << RESTendl;
        }
        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "RawSignalIdTagging"; }

    TRestRawSignalIdTaggingProcess();   // Constructor
    ~TRestRawSignalIdTaggingProcess();  // Destructor

    ClassDefOverride(TRestRawSignalIdTaggingProcess, 3);
};
#endif
