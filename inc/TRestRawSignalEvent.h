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

#ifndef RestDAQ_TRestRawSignalEvent
#define RestDAQ_TRestRawSignalEvent

#include <TAxis.h>
#include <TGraph.h>
#include <TPad.h>
#include <TRestEvent.h>
#include <TRestRawReadoutMetadata.h>
#include <TRestRun.h>
#include <TVector2.h>

#include <iostream>
#include <string>

#include "TRestRawSignal.h"

//! An event container for time rawdata signals with fixed length
class TRestRawSignalEvent : public TRestEvent {
   protected:
    TGraph* gr;          //!
    Double_t fMinTime;   //!
    Double_t fMaxTime;   //!
    Double_t fMinValue;  //!
    Double_t fMaxValue;  //!

    TVector2 fBaseLineRange = TVector2(-1, -1);  //!
    TVector2 fRange = TVector2(-1, -1);          //!

    std::vector<TRestRawSignal> fSignal;  // Collection of signals that define the event

   private:
    void SetMaxAndMin();

   public:
    Bool_t signalIDExists(Int_t sID) {
        if (GetSignalIndex(sID) == -1) return false;
        return true;
    }

    std::vector<TRestRawSignal> GetSignals() const { return fSignal; }

    // Setters
    void AddSignal(TRestRawSignal& s);

    void RemoveSignalWithId(Int_t sId);

    void AddChargeToSignal(Int_t sgnlID, Int_t bin, Short_t value);

    void SetTailPoints(Int_t p) {
        for (int n = 0; n < GetNumberOfSignals(); n++) fSignal[n].SetTailPoints(p);
    }

    // Set and Get for BiPo
    void SetAuxiliar(Double_t aux);
    auto GetAuxiliar();

    /// It sets the range to be used for the baseline calculation and calls
    /// TRestRawSignal::CalculateBaseLine()
    void SetBaseLineRange(TVector2 blRange, std::string option = "") {
        SetBaseLineRange(blRange.X(), blRange.Y(), option);
    }

    /// It sets the range to be used for the baseline calculation and calls
    /// TRestRawSignal::CalculateBaseLine()
    void SetBaseLineRange(Int_t from, Int_t to, std::string option = "") {
        fBaseLineRange = TVector2(from, to);
        for (int n = 0; n < GetNumberOfSignals(); n++) fSignal[n].CalculateBaseLine(from, to, option);
    }

    void SetRange(TVector2 range) { SetRange(range.X(), range.Y()); }

    void SetRange(Int_t from, Int_t to) {
        fRange = TVector2(from, to);
        for (int n = 0; n < GetNumberOfSignals(); n++) fSignal[n].SetRange(fRange);
    }

    // Getters
    inline Int_t GetNumberOfSignals() const { return fSignal.size(); }
    TRestRawSignal* GetSignal(Int_t n) { return &fSignal[n]; }

    void PrintSignalIds() {
        for (int n = 0; n < GetNumberOfSignals(); n++) {
            if (n > 0) std::cout << " , ";
            std::cout << GetSignal(n)->GetSignalID();
        }
        std::cout << std::endl;
    }

    std::vector<int> GetSignalIds() {
        std::vector<int> ids;
        for (int n = 0; n < GetNumberOfSignals(); n++) ids.push_back(GetSignal(n)->GetSignalID());
        return ids;
    }

    Bool_t isBaseLineInitialized() {
        // If one signal is initialized we assume initialization happened for any signal
        for (int n = 0; n < GetNumberOfSignals(); n++)
            if (fSignal[n].isBaseLineInitialized()) return true;
        return false;
    }

    TRestRawSignal* GetSignalById(Int_t sid) {
        Int_t index = GetSignalIndex(sid);
        if (index < 0) return nullptr;

        return &fSignal[index];
    }

    TRestRawSignalEvent GetSignalEventForType(const std::string& type) const;
    TRestRawSignalEvent GetSignalEventForTypes(
        const std::set<std::string>& types, const TRestRawReadoutMetadata* readoutMetadata = nullptr) const;

    TRestRawSignal* GetMaxSignal();

    Int_t GetLowestWidth(Double_t minPeakAmplitude = 0);
    Double_t GetLowAverageWidth(Int_t nSignals = 5, Double_t minPeakAmplitude = 0);
    Double_t GetAverageWidth(Double_t minPeakAmplitude = 0);

    Int_t GetSignalIndex(Int_t signalID);

    Double_t GetBaseLineAverage();
    Double_t GetBaseLineSigmaAverage();
    //   void SubtractBaselines();
    Double_t GetIntegral();
    Double_t GetThresholdIntegral();

    Double_t GetSlopeIntegral();
    Double_t GetRiseSlope();
    Double_t GetRiseTime();
    Double_t GetTripleMaxIntegral();

    Double_t GetMaxValue();
    Double_t GetMinValue();
    Double_t GetMinTime();
    Double_t GetMaxTime();

    // Default
    void Initialize();
    void PrintEvent();

    TPad* DrawEvent(const TString& option = "");
    void DrawSignals(TPad* pad, const std::vector<Int_t>& signals);
    TPad* DrawSignal(Int_t signalID, const TString& option = "");

    TRestRawReadoutMetadata* GetReadoutMetadata() const;

    // Constructor
    TRestRawSignalEvent();
    // Destructor
    virtual ~TRestRawSignalEvent();

    ClassDef(TRestRawSignalEvent, 1);
};
#endif
