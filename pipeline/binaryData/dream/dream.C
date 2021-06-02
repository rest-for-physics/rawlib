
Int_t dream() {
    TRestRun* run = new TRestRun("dream_signals.root");

    TRestRawSignalEvent* ev = (TRestRawSignalEvent*)run->GetInputEvent();

    run->GetEntry(0);

    if (ev->GetNumberOfSignals() != 512) {
        cout << "Number of signals is not 512! " << endl;
        return 1;
    }

    if (ev->GetSignal(0)->GetMaxPeakBin() != 188) {
        cout << "Max peak position of signal 0 is not 188!" << endl;
        return 2;
    }

    if (ev->GetSignal(0)->GetMaxPeakValue() != 494) {
        cout << "Max peak value of signal 0 is not 494!" << endl;
        return 2;
    }

    if (ev->GetSignal(100)->GetMaxPeakBin() != 154) {
        cout << "Max peak position of signal 0 is not 154!" << endl;
        return 3;
    }

    if (ev->GetSignal(100)->GetMaxPeakValue() != 500) {
        cout << "Max peak value of signal 0 is not 500!" << endl;
        return 4;
    }

    if (ev->GetSignal(511)->GetNumberOfPoints() != 200) {
        cout << "Number of points in last signal is not 200!" << endl;
        return 5;
    }

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
