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

#ifndef RestCore_TRestRawSignalConvolutionFittingProcess
#define RestCore_TRestRawSignalConvolutionFittingProcess

#include <TRestRawSignalEvent.h>

#include "TF1Convolution.h"
#include "TH1D.h"
#include "TRestEventProcess.h"

//! An analysis REST process to extract valuable information from RawSignal type
//! of data.
class TRestRawSignalConvolutionFittingProcess : public TRestEventProcess {
   private:
    /// A pointer to the specific TRestRawSignalEvent input
    TRestRawSignalEvent* fRawSignalEvent;  //!

    /* Metadata members
  TVector2 fBaseLineRange;
  TVector2 fIntegralRange;
  Double_t fPointThreshold;
  Double_t fSignalThreshold;
  Int_t fNPointsOverThreshold; */

    void InitFromConfigFile() override;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fRawSignalEvent; }
    RESTValue GetOutputEvent() const override { return fRawSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        /*
    metadata << "Baseline range : ( " << fBaseLineRange.X() << " , " <<
    fBaseLineRange.Y() << " ) "
         << endl;
    metadata << "Integral range : ( " << fIntegralRange.X() << " , " <<
    fIntegralRange.Y() << " ) " << endl;
    metadata << "Point Threshold : " << fPointThreshold << " sigmas" << endl;
    metadata << "Signal threshold : " << fSignalThreshold << " sigmas" << endl;
    metadata << "Number of points over threshold : " << fNPointsOverThreshold << endl;
    metadata << " " << endl;
        */

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "rawSignalConvolutionFitting"; }

    TRestRawSignalConvolutionFittingProcess();  // Constructor
    TRestRawSignalConvolutionFittingProcess(const char* configFilename);
    ~TRestRawSignalConvolutionFittingProcess();  // Destructor

    ClassDefOverride(TRestRawSignalConvolutionFittingProcess, 1);
};
#endif
