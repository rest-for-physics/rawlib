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
/// The TRestRawSignalChannelActivityProcess allows to generate different
/// histograms in order to monitor the number of times a channel has observed
/// a raw signal given a set of conditions on the threshold and number of active
/// channels.
///
/// TRestRawSignalChannelActivityProcess produces different channel activity
/// histograms involving raw signals. These histograms thus show the activity
/// of the channels before zero suppression, which can be useful to perform
/// noise studies.
///
/// The following list describes the different histograms that are generated:
///
/// * **daqChannelActivityRaw**: Histogram based on the DAQ channels.
/// The following figure shows the DAQ channel activity histogram for raw
/// signals in:
///     * a) the case where all the channels are saved (where a flat
///          distribution is seen because all channels have the same raw
///          activity)
///     * b) the case where only channels that have been hit are saved.
///
/// \htmlonly <style>div.image img[src="daqChActRaw.png"]{width:1000px;}</style>
/// \endhtmlonly
/// ![An illustration of the daq raw signals channel activity](daqChActRaw.png)
///
/// * **rChannelActivityRaw**: histogram based on the readout channels, i.e.,
/// after converting the daq channel numbering into readout channel numbering
/// based on the .dec file.
///
/// * **rChannelActivityRaw_N**: where *N* can be 1, 2, 3 or M (multi), is
/// a histogram based on the readout channels, i.e., after converting the daq
/// channel numbering into readout channel numbering based on the .dec
/// file, that contains the events with *N* number of signals above the
/// **lowThreshold**
/// set by the user.
///
/// * **rChannelActivityRaw_NH**: where *N* can be 1, 2, 3 or M (multi), is
/// a histogram based on the readout channels, i.e., after converting the daq
/// channel numbering into readout channel numbering based on the .dec
/// file, that contains the events with *N* number of signals above the
/// **highThreshold**
/// set by the user.
///
/// The number of channels and their numbering can be specified by the user to
/// match the detector being used.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
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
/// 2020-August: First implementation of raw signal channel activity process.
///              Cristina Margalejo
///
/// \class      TRestRawSignalChannelActivityProcess
/// \author     Cristina Margalejo
///
/// <hr>
///
#include "TRestRawSignalChannelActivityProcess.h"

using namespace std;

ClassImp(TRestRawSignalChannelActivityProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalChannelActivityProcess::TRestRawSignalChannelActivityProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalChannelActivityProcess::~TRestRawSignalChannelActivityProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalChannelActivityProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fSignalEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization. The ROOT TH1 histograms are created here
/// using
/// the limits defined in the process metadata members.
///
/// The readout histograms will only be created in case an appropriate readout
/// definition
/// is found in the processing chain.
///
void TRestRawSignalChannelActivityProcess::InitProcess() {
    if (!fReadOnly) {
        fDaqChannelsHisto = new TH1D("daqChannelActivityRaw", "daqChannelActivityRaw", fDaqChannels,
                                     fDaqStartChannel, fDaqEndChannel);
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalChannelActivityProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fSignalEvent->InitializeReferences(run);
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fSignalEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelType.empty()) {
        cerr << "TRestRawSignalChannelActivityProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    for (int s = 0; s < fSignalEvent->GetNumberOfSignals(); s++) {
        const auto signal = fSignalEvent->GetSignal(s);
        if (!fChannelType.empty()) {
            const auto channelType = fReadoutMetadata->GetTypeForChannelDaqId(signal->GetID());
            if (fChannelType != channelType) {
                continue;
            }
        }
        // Adding signal to the channel activity histogram
        if (!fReadOnly) {
            Int_t daqChannel = signal->GetID();
            fDaqChannelsHisto->Fill(daqChannel);
        }
    }

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug)
        fAnalysisTree->PrintObservables();

    return fSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. In this process it will take care of writing the histograms
/// to disk.
///
void TRestRawSignalChannelActivityProcess::EndProcess() {
    if (!fReadOnly) {
        fDaqChannelsHisto->Write();
    }
}
