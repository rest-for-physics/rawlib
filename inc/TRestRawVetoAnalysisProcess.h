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

#ifndef RestCore_TRestRawVetoAnalysisProcess
#define RestCore_TRestRawVetoAnalysisProcess

#include "TRestEventProcess.h"
#include "TRestRawSignalEvent.h"

//! A process that allows to define several signal IDs as veto channels.
//! The data from the vetoes is then removed from the events and stored
//! as separate observables.
class TRestRawVetoAnalysisProcess : public TRestEventProcess {
   private:
    /// The range used to calculate the baseline parameters from the veto signal
    TVector2 fBaseLineRange;  //<

    /// The range used to calculate the veto signal parameters
    TVector2 fRange;  //<

    /// Threshold of the vetoes
    Int_t fThreshold;

    /// Peak time window for cut
    vector<double> fTimeWindow;

    /// Veto signal IDs
    vector<double> fVetoSignalId;

    /// Veto signal IDs per group
    vector<string> fVetoGroupIds;

    /// Veto group Names
    vector<string> fVetoGroupNames;

    /// Peak Time observable names
    vector<string> fPeakTime;
    /// Max peak amplitude observable names
    vector<string> fPeakAmp;

    /// A pointer to the specific TRestRawSignalEvent
    TRestRawSignalEvent* fSignalEvent;  //!

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
   public:
    any GetInputEvent() { return fSignalEvent; }
    any GetOutputEvent() { return fSignalEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* evInput);

    void LoadConfig(std::string cfgFilename, std::string name = "");

    void PrintMetadata();

    /// Returns a new instance of this class
    TRestEventProcess* Maker() { return new TRestRawVetoAnalysisProcess; }

    /// Returns the name of this process
    TString GetProcessName() { return (TString) "vetoAnalysis"; }

    /// Returns the veto IDs, if they where defined in a list
    std::vector<double> GetVetoSignalIDs() { return fVetoSignalId; }
    double GetVetoSignalIDs(Int_t index) {
        if (index >= fVetoSignalId.size()) return -1;
        return fVetoSignalId[index];
    }

    /// Returns the veto group names and IDs
    std::pair<vector<string>, vector<string>> GetVetoGroups() {
        pair<vector<string>, vector<string>> output;
        output.first = fVetoGroupNames;
        output.second = fVetoGroupIds;
        return output;
    }

    Int_t GetGroupIndex(std::string groupName);
    string GetGroupIds(std::string groupName);

    TRestRawVetoAnalysisProcess();
    TRestRawVetoAnalysisProcess(char* cfgFileName);

    ~TRestRawVetoAnalysisProcess();

    // If new members are added, removed or modified in this class version number
    // must be increased!
    ClassDef(TRestRawVetoAnalysisProcess, 1);
};
#endif
