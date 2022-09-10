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
/// Process to identify events from different DAQ id ranges.
/// For example, it might be used to identify different detectors, modules or
/// regions of the detectors that are read with a common acquisition setup.
///
/// This process was motivated by the TREX-DM experiment, where we have a
/// readout with two independent readout planes. This process will allow to
/// identify events happening at any of those two planes in an early data
/// processing stage, where we do not have yet access to the
/// TRestDetectorReadout description.
///
/// In the particular case of TREX-DM, we define two readout planes as a
/// function of the daq id range as follows:
///  - South detector 0 to 575 IDs
///  - North detector 576 to 1151 IDs.
///
/// This process allows to define a `tag` associated to given id range. Each
/// tag contains the user given `name` and the associated range of ids.
///
/// Metadata parameters that can be defined inside the `<tag` definition:
/// * **name**: Name for the range.
/// * **ids**: Range of daq IDs.
///
/// Any number of daq ID ranges can be defined (e.g. as many "tag" sections as needed).
///
/// Example in rml file:
/// \code
/// <addProcess type="TRestRawSignalIdTaggingProcess" name="TREXsides" value="ON" observable="all" >
///     <tag name="South" ids="(0,575)"/>
///     <tag name="North" ids="(576,1151)"/>
/// </addProcess>
/// \endcode
///
/// Each tag is associated with an integer number, from 1 upwards, in the order found
/// inside the `tag` list. First tag being associated to 1, second tag to 2, etc. This
/// will be used to construct an observable helping to identify the tags it belongs to.
///
///  ### Observables
///
/// * **tagId**: Each digit corresponds to a daq ID range activated in the event,
/// ordered with increasing order.
/// As an example, in previous rml there are 3 possible values for this observable:
///     * 1: Channels were found inside the `South` taq id range definition.
///     * 2: Channels were found inside the `North` taq id range definition.
///     * 12: Channels were found on both, `North` and `South` definitions.
///
/// ### Cuts
///
/// This process implements TRestEventProcess::ApplyCut, therefore, we might apply
/// a selection of events to get only those events that belong to a given definition.
///
/// To keep only "South" events for further data processing we would do:
/// \code
/// <cut name="tagId" value="(1,1)" />
/// \endcode
///
/// To keep only those events producing events on both detectors we could do:
/// \code
/// <cut name="tagId" value="(10,20)" />
/// \endcode
//
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
    SetObservableValue("tagId", result);

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    return fSignalEvent;
}
