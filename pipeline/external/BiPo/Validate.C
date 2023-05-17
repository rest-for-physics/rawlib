Int_t Validate() {
    TRestRun run("RawData_BiPo3Mod2_02600.root");

    TRestRawBiPoToSignalProcess* bipo =
        (TRestRawBiPoToSignalProcess*)run.GetMetadataClass("TRestRawBiPoToSignalProcess");

    int checksum = bipo->GetMatacqBoard(9).address;
    checksum += bipo->GetMatacqBoard(9).en_ch[0] + bipo->GetMatacqBoard(9).en_ch[1] +
                bipo->GetMatacqBoard(9).en_ch[2] + bipo->GetMatacqBoard(9).en_ch[3];
    checksum += bipo->GetMatacqBoard(9).trg_ch[0] + bipo->GetMatacqBoard(9).trg_ch[1] +
                bipo->GetMatacqBoard(9).trg_ch[2] + bipo->GetMatacqBoard(9).trg_ch[3];

    checksum += bipo->GetMatacqBoard(9).Trig_Type;
    checksum += bipo->GetMatacqBoard(9).Threshold;
    checksum += bipo->GetMatacqBoard(9).Nb_Acq;
    checksum += bipo->GetMatacqBoard(9).Posttrig;
    checksum += bipo->GetMatacqBoard(9).Time_Tag_On;
    checksum += bipo->GetMatacqBoard(9).Sampling_GHz;

    std::cout << "Checksum: " << checksum << std::endl;
    if (checksum != 145) {
        std::cout << "Error. Matacq settings of the latest board seem to be wrong!" << std::endl;
        bipo->PrintMetadata();
        return 1;
    }

    checksum = bipo->GetBiPoSettings(9).trigger_address;
    checksum += bipo->GetBiPoSettings(9).Win1_Posttrig;
    checksum += bipo->GetBiPoSettings(9).Timeout_200KHz;
    checksum += bipo->GetBiPoSettings(9).Trig_Chan[0] + bipo->GetBiPoSettings(9).Trig_Chan[1] +
                bipo->GetBiPoSettings(9).Trig_Chan[2] + bipo->GetBiPoSettings(9).Trig_Chan[3];
    checksum += bipo->GetBiPoSettings(9).Level1_mV[0] + bipo->GetBiPoSettings(9).Level1_mV[1] +
                bipo->GetBiPoSettings(9).Level1_mV[2] + bipo->GetBiPoSettings(9).Level1_mV[3];
    checksum += bipo->GetBiPoSettings(9).Level2_mV[0] + bipo->GetBiPoSettings(9).Level2_mV[1] +
                bipo->GetBiPoSettings(9).Level2_mV[2] + bipo->GetBiPoSettings(9).Level2_mV[3];

    checksum += bipo->GetBiPoSettings(9).t1_window;
    checksum += bipo->GetBiPoSettings(9).t2_window;
    checksum += bipo->GetBiPoSettings(9).t1_t2_timeout;

    std::cout << "Checksum: " << checksum << std::endl;
    if (checksum != 3748) {
        std::cout << "Error. BiPo settings of the latest board seem to be wrong!" << std::endl;
        bipo->PrintMetadata();
        return 2;
    }

    TRestEvent* ev = (TRestEvent*)run.GetInputEvent();
    run.GetEntry(4);

    std::cout.precision(12);
    std::cout << "Time stamp: " << round(100. * ev->GetTime()) / 100. << std::endl;
    if (round(100. * ev->GetTime()) != 137550274597) {
        std::cout << "A problem occurred reading the event time stamp!" << std::endl;
        return 3;
    }

    return 0;
}
