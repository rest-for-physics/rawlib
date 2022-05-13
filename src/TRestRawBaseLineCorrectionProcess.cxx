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
 /////////////////////////////////////////////////////////////////////////
 /// A process that corrects the BaseLine. The parameter "SmoothingWindow" sets the time window (number of bins) used for the moving average filter that is used for the baseline correction. Standard value is 75.
 /// The range of signal IDs, to which the process is applied, can be selected with the parameter "SignalsRange". The process can be added to the analysis rml for example like
 /// \code
 ///   <addProcess type="TRestRawBaseLineCorrectionProcess" name="blCorrection" value="ON" verboseLevel="silent">
 ///       <parameter name="signalsRange" value="(4610,4900)" />
 ///       <parameter name="smoothingWindow" value="75" />
 ///   </addProcess>
 /// \endcode
 /// \htmlonly <style>div.image img[src="doc_TRestRawBaseLineCorrectionProcess1.png"]{width:750px;}</style> \endhtmlonly
 ///
 /// ![A raw signal with and without TRestBaseLineCorrectionProcess with smoothing window size 75](doc_TRestRawBaseLineCorrectionProcess1.png) 
 /// 
 /// \htmlonly <style>div.image img[src="doc_TRestRawBaseLineCorrectionProcess2.png"]{width:750px;}</style> \endhtmlonly
 ///
 /// ![A raw signal with and without TRestBaseLineCorrectionProcess with smoothing window size 75](doc_TRestRawBaseLineCorrectionProcess2.png) 
 /// <hr>
 ///
 /// \warning **⚠ REST is under continous development.** This documentation
 /// is offered to you by the REST community. Your HELP is needed to keep this code
 /// up to date. Your feedback will be worth to support this software, please report
 /// any problems/suggestions you may find while using it at [The REST Framework
 /// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
 /// updating
 /// information or adding/proposing new contributions. See also our
 /// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
 /// Guide</a>.
 ///
 ///
 ///--------------------------------------------------------------------------
 ///
 /// RESTsoft - Software for Rare Event Searches with TPCs
 ///
 /// History of developments:
 ///
 /// 2022-Mar:  First implementation
 ///             Konrad Altenmueller                                                                
 /// \class TRestRawBaseLineCorrectionProcess                                             
 /// \author     Konrad Altenmueller
 ///                                                                      
 /// <hr>                                                                 
 /////////////////////////////////////////////////////////////////////////

#include "TRestRawBaseLineCorrectionProcess.h"

ClassImp(TRestRawBaseLineCorrectionProcess);

TRestRawBaseLineCorrectionProcess::TRestRawBaseLineCorrectionProcess() {
    Initialize();
}

TRestRawBaseLineCorrectionProcess::~TRestRawBaseLineCorrectionProcess() {
    delete fInputEvent;
    delete fOutputEvent;
}

void TRestRawBaseLineCorrectionProcess::Initialize() {
    SetSectionName(this->ClassName());
    fInputEvent = NULL;
    fOutputEvent = new TRestRawSignalEvent();
}

void TRestRawBaseLineCorrectionProcess::InitProcess() {
    if (fSignalsRange.X() != -1 && fSignalsRange.Y() != -1) fRangeEnabled = true;
}

TRestEvent* TRestRawBaseLineCorrectionProcess::ProcessEvent(TRestEvent * evInput) {

        fInputEvent = (TRestRawSignalEvent*)evInput;

        for (int s = 0; s < fInputEvent->GetNumberOfSignals(); s++) {
            TRestRawSignal* sgnl = fInputEvent->GetSignal(s);

            if (fRangeEnabled && (sgnl->GetID() < fSignalsRange.X() || sgnl->GetID() > fSignalsRange.Y())){
                fOutputEvent->AddSignal(*sgnl);
                continue;
            }

            TRestRawSignal sgnl2( ) ;
            sgnl->GetBaseLineCorrected(&sgnl2, fSmoothingWindow);
            sgnl2.SetID( sngl->GetID( ) );
            fOutputEvent->AddSignal( sgnl2 );
        }
    
    return fOutputEvent;
}


void TRestRawBaseLineCorrectionProcess::EndProcess() {
    }

