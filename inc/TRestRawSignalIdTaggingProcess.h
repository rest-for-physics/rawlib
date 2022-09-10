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

//! An analysis process to extract valuable information from a TRestRawSignalEvent.
class TRestRawSignalIdTaggingProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fSignalEvent;  //!

    std::vector<std::string> fTagNames;
    std::vector<TVector2> fIdRanges;

    void Initialize() override;
    void InitFromConfigFile() override;

   protected:
    // add here the members of your event process

   public:
    any GetInputEvent() const override { return fSignalEvent; }
    any GetOutputEvent() const override { return fSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Tag code: " << RESTendl;
        for (int n = 0; n < fIdRanges.size(); n++) {
            RESTMetadata << n + 1 << " - " << fTagNames[n] << ": ( " << fIdRanges[n].X() << ", "
                         << fIdRanges[n].Y() << " )" << RESTendl;
        }
        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "RawSignalIdTagging"; }

    TRestRawSignalIdTaggingProcess();   // Constructor
    ~TRestRawSignalIdTaggingProcess();  // Destructor

    ClassDefOverride(TRestRawSignalIdTaggingProcess, 3);
};
#endif
