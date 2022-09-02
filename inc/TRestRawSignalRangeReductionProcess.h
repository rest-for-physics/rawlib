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

#ifndef RestCore_TRestRawSignalRangeReductionProcess
#define RestCore_TRestRawSignalRangeReductionProcess

#include <TRestEventProcess.h>

#include "TRestRawSignalEvent.h"

//! A process to reduce the range of values of the signals to emulate a realistic ADC.
//! Using Short_t (default) is equivalent to using a 16 bit ADC, we can use this process to go from a 16 bit
//! signal to a 12 bit signal (between 0 and 4095) for example.
class TRestRawSignalRangeReductionProcess : public TRestEventProcess {
   private:
    TRestRawSignalEvent* fInputSignalEvent;
    TRestRawSignalEvent* fOutputSignalEvent;

    void Initialize() override;

    void LoadDefaultConfig();

    UShort_t fResolutionInBits = 12;  // from 1 to 16 bits
    TVector2 fDigitizationInputRange =
        TVector2(std::numeric_limits<Short_t>::min(), std::numeric_limits<Short_t>::max());

   public:
    inline Double_t GetResolutionInNumberOfBits() const { return fResolutionInBits; }
    void SetResolutionInNumberOfBits(UShort_t nBits);

    inline TVector2 GetDigitizationInputRange() const { return fDigitizationInputRange; }
    void SetDigitizationInputRange(const TVector2& range);

    any GetInputEvent() const override { return fInputSignalEvent; }
    any GetOutputEvent() const override { return fOutputSignalEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    inline void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Resolution in bits: " << fResolutionInBits << RESTendl;
        RESTMetadata << "Digitization range: (" << fDigitizationInputRange.X() << ", "
                     << fDigitizationInputRange.Y() << ")" << RESTendl;

        EndPrintProcess();
    }

    TRestMetadata* GetProcessMetadata() const { return nullptr; }

    const char* GetProcessName() const override { return "rawSignalRangeReductionProcess"; }

    // Constructor
    TRestRawSignalRangeReductionProcess();
    TRestRawSignalRangeReductionProcess(const char* configFilename);
    // Destructor
    ~TRestRawSignalRangeReductionProcess();

    ClassDefOverride(TRestRawSignalRangeReductionProcess, 1);
};
#endif  // RestCore_TRestRawSignalRangeReductionProcess
