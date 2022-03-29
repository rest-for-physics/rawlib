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

#ifndef RestCore_TRestRawSignalFitEventProcess
#define RestCore_TRestRawSignalFitEventProcess

#include <TRestRawSignalEvent.h>

#include "TF1Convolution.h"
#include "TH1D.h"
#include "TMath.h"
#include "TRestEventProcess.h"

//! An analysis REST process to extract valuable information from RawSignal type of data.
class TRestRawSignalFitEventProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fRawSignalEvent;  //!

    // parameters
    TVector2 fBaseLineRange = TVector2(0, 0);
    Double_t fPointThreshold = 0;
    Double_t fSignalThreshold = 0;
    Int_t fPointsOverThreshold = 0;
    Bool_t fAgetFit = false;
    Bool_t fAddAllPulses = false;

    Double_t fShapingFixed = 0;
    Double_t fStartPositionFixed = 0;
    Double_t fVarianceFixed = 0;
    Double_t fAmplitudeFixed = 0;

    Double_t fShapingInitialValue = 0;
    Double_t fStartPositionInitialValue = 0;
    Double_t fVarianceInitialValue = 0;
    Double_t fAmplitudeInitialValue = 0;

    void Initialize();

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    any GetInputEvent() { return fRawSignalEvent; }
    any GetOutputEvent() { return fRawSignalEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void LoadConfig(std::string cfgFilename, std::string name = "");

    void PrintMetadata() {
        BeginPrintProcess();

        metadata << "Baseline range : ( " << fBaseLineRange.X() << " , " << fBaseLineRange.Y() << " ) "
                 << endl;
        metadata << "Point Threshold : " << fPointThreshold << " sigmas" << endl;
        metadata << "Signal threshold : " << fSignalThreshold << " sigmas" << endl;
        metadata << "Number of points over threshold : " << fPointsOverThreshold << endl;
        metadata << " " << endl;

        if (fShapingFixed != 0) {
            metadata << "Shaping fixed : " << fShapingFixed << endl;
        }
        if (fStartPositionFixed != 0) {
            metadata << "Start position fixed : " << fStartPositionFixed << endl;
        }
        if (fVarianceFixed != 0) {
            metadata << "Variance gauss fixed : " << fVarianceFixed << endl;
        }
        if (fAmplitudeFixed != 0) {
            metadata << "Amplitude gauss fixed : " << fAmplitudeFixed << endl;
        }

        if (fShapingInitialValue != 0) {
            metadata << "Shaping initial value : " << fShapingInitialValue << endl;
        }
        if (fStartPositionInitialValue != 0) {
            metadata << "Start position initial value : " << fStartPositionInitialValue << endl;
        }
        if (fVarianceInitialValue != 0) {
            metadata << "Variance gauss initial value : " << fVarianceInitialValue << endl;
        }
        if (fAmplitudeInitialValue != 0) {
            metadata << "Amplitude gauss initial value : " << fAmplitudeInitialValue << endl;
        }

        if (fAgetFit) {
            metadata << "Fitting mode : AGET" << endl;
        }
        if (!fAgetFit) {
            metadata << "Fitting mode : Convolution" << endl;
        }

        if (fAddAllPulses) {
            metadata << "Adding all pulses in the event" << endl;
        }

        EndPrintProcess();
    }

    TString GetProcessName() { return (TString) "rawSignalFitEvent"; }

    TRestRawSignalFitEventProcess();  // Constructor
    TRestRawSignalFitEventProcess(char* cfgFileName);
    ~TRestRawSignalFitEventProcess();  // Destructor

    ClassDef(TRestRawSignalFitEventProcess, 3);
};
#endif
