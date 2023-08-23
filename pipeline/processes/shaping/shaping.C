Int_t shaping(Bool_t draw = false) {
    TRestRawSignalEvent* ev = new TRestRawSignalEvent();

    TRestRawSignal* signal = new TRestRawSignal(512);

    signal->IncreaseBinBy(170, 100);
    signal->IncreaseBinBy(250, 250);

    ev->AddSignal(*signal);

    // Initializing processes
    string cfgFile1 = "gaus.rml";
    TRestRawSignalShapingProcess* shaper1 = new TRestRawSignalShapingProcess((char*)cfgFile1.c_str());
    shaper1->PrintMetadata();

    string cfgFile2 = "shaper.rml";
    TRestRawSignalShapingProcess* shaper2 = new TRestRawSignalShapingProcess((char*)cfgFile2.c_str());
    shaper2->PrintMetadata();

    TRestRawSignalEvent* gausEvent = (TRestRawSignalEvent*)shaper1->ProcessEvent(ev);
    TRestRawSignalEvent* agetEvent = (TRestRawSignalEvent*)shaper2->ProcessEvent(ev);

    agetEvent->GetSignal(0)->SetRangeToMax();
    gausEvent->GetSignal(0)->SetRangeToMax();

    Double_t gausMaxPeakPosition = gausEvent->GetSignal(0)->GetMaxPeakBin();
    Double_t gausMaxPeakValue = gausEvent->GetSignal(0)->GetMaxPeakValue();
    Double_t agetMaxPeakPosition = agetEvent->GetSignal(0)->GetMaxPeakBin();
    Double_t agetMaxPeakValue = agetEvent->GetSignal(0)->GetMaxPeakValue();

    cout << "Gaus Max Peak Position: " << gausMaxPeakPosition << endl;
    cout << "Gaus Max Peak Value: " << gausMaxPeakValue << endl;
    cout << "AGET Max Peak Position: " << agetMaxPeakPosition << endl;
    cout << "AGET Max Peak Value: " << agetMaxPeakValue << endl;

    /* Used to generate a combined plot */
    if (draw) {
        TRestRawSignalEvent* combinedEv = new TRestRawSignalEvent();

        TRestRawSignal signal1 = *ev->GetSignal(0);
        signal1.SetID(0);
        combinedEv->AddSignal(signal1);

        TRestRawSignal signal2 = *gausEvent->GetSignal(0);
        signal2.SetID(1);
        combinedEv->AddSignal(signal2);

        TRestRawSignal signal3 = *agetEvent->GetSignal(0);
        signal3.SetID(2);
        combinedEv->AddSignal(signal3);

        TCanvas* c = new TCanvas("c0", "", 800, 600);
        combinedEv->DrawEvent();
    }

    if (gausMaxPeakPosition != 250) {
        cout << "Problem on gaussian convolution! Position of the most intense peak should be 250!!" << endl;
        return 1;
    }

    if (gausMaxPeakValue != 200) {
        cout << "Problem on gaussian convolution! Amplitude of the most intense peak should be 200!!" << endl;
        return 2;
    }

    if (agetMaxPeakPosition != 289) {
        cout << "Problem on shaperSin convolution! Position of the most intense peak should be 289!!" << endl;
        return 3;
    }

    if (agetMaxPeakValue != 173) {
        cout << "Problem on shaperSin convolution! Amplitude of the most intense peak should be 173!!"
             << endl;
        return 4;
    }

    delete shaper1, shaper2;
    delete signal;
    delete ev;

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
