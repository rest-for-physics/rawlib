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

#ifndef RestCore_TRestRawSignalViewerProcess
#define RestCore_TRestRawSignalViewerProcess

#include <TH1D.h>
#include <TRestEventProcess.h>
#include <TRestRawSignalEvent.h>

//! A generic viewer REST process to visualize raw signals and
//! parameters obtained from the anlysisTree on the processes canvas.
class TRestRawSignalViewerProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fSignalEvent;  //!

    std::vector<TObject*> fDrawingObjects;  //!
    Double_t fDrawRefresh;                  //!

    TVector2 fBaseLineRange;  //!
    int eveCounter = 0;       //!
    int sgnCounter = 0;       //!

    TPad* DrawSignal(Int_t signal);
    TPad* DrawObservables();

    void InitFromConfigFile();

    void Initialize();

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
   inline any GetInputEvent() const { return fSignalEvent; }
   inline any GetOutputEvent() const { return fSignalEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string configFilename, std::string name = "");

    void PrintMetadata() {
        BeginPrintProcess();

        std::cout << "Refresh value : " << fDrawRefresh << endl;

        EndPrintProcess();
    }

    inline TString GetProcessName() const { return (TString) "rawSignalViewer"; }

    // Constructor
    TRestRawSignalViewerProcess();
    TRestRawSignalViewerProcess(char* configFilename);
    // Destructor
    ~TRestRawSignalViewerProcess();

    ClassDef(TRestRawSignalViewerProcess, 1);
};
#endif
