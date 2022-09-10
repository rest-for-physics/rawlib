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
//////////////////////////////////////////////////////////////////////////
///
/// Process to identify events from different Daq Id Ranges.
/// For example in cases where several detectors are read with same daq.
/// This is happens in TREX-DM: South detector 0 to 575 IDs
/// North detector 576 to 1151 IDs.
///
/// Metadata parameters that can be defined in the rml:
/// * **name**: Name for the range.
/// * **ids**: Range of daq IDs.
///
/// Each range must be defined under the "tag" section.
/// Any number of daq ID ranges can be defined (e.g. as many "tag" sections as needed).
/// Numbers are assigned for each range, from 1 upwards.
///
/// Example in rml file:
/// \code
/// <addProcess type="TRestRawSignalIdTaggingProcess" name="TREXsides" value="ON" observable="all" >
///     <tag name="South" ids="(0,575)"/>
///     <tag name="North" ids="(576,1151)"/>
/// </addProcess>
/// \endcode
///
///  ### Observables
///
/// * **DaqIdRanges**: Each digit corresponds a daq ID range activated in the event,
/// ordered with increasing order.
/// As an example, in previous rml there are 3 possible values for this observable:
///     * 1: South daq ID range.
///     * 2: North daq ID range.
///     * 12: South and North daq ID ranges.
///
///_______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2022-September: First implementation of TRestRawSignalIdTaggingProcess
///                 Created from TRestRawSignalAnalysisProcess
///
/// \class      TRestRawSignalIdTaggingProcess
/// \author     David Díez Ibáñez
///
/// <hr>
///

#include "TRestRawSignalIdTaggingProcess.h"

using namespace std;

ClassImp(TRestRawSignalIdTaggingProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalIdTaggingProcess::TRestRawSignalIdTaggingProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalIdTaggingProcess::~TRestRawSignalIdTaggingProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalIdTaggingProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fSignalEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalIdTaggingProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalIdTaggingProcess::InitFromConfigFile() {
    // This line is to exploit the retrieval of parameter as it is done at any process
    TRestEventProcess::InitFromConfigFile();

    // This is the additional code required by the process to read tags
    TiXmlElement* tagDefinition = GetElement("tag");
    while (tagDefinition) {
        fTagNames.push_back(GetFieldValue("name", tagDefinition));
        fIdRanges.push_back(Get2DVectorParameterWithUnits("ids", tagDefinition));

        tagDefinition = GetNextElement(tagDefinition);
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalIdTaggingProcess::ProcessEvent(TRestEvent* evInput) {
    fSignalEvent = (TRestRawSignalEvent*)evInput;

    std::vector<int> tagCode;

    for (int j = 0; j < fSignalEvent->GetNumberOfSignals(); j++) {
        TRestRawSignal* singleSignal = fSignalEvent->GetSignal(j);

        for (int n = 0; n < fIdRanges.size(); n++) {
            if (singleSignal->GetID() >= fIdRanges[n].X() && singleSignal->GetID() <= fIdRanges[n].Y()) {
                // If it is not already in the vector, adds it. n+1 to avoid 0.
                if (std::find(tagCode.begin(), tagCode.end(), n + 1) == tagCode.end()) {
                    tagCode.push_back(n + 1);
                }
            }
        }
    }
    std::sort(tagCode.begin(), tagCode.end());

    int result = 0;
    for (auto d : tagCode) {
        result = result * 10 + d;
    }
    SetObservableValue("DaqIdRanges", result);

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    return fSignalEvent;
}
