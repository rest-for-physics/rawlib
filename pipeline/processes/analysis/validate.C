Int_t validate() {
    TRestRun* run = new TRestRun("R01208_output.root");

    TRestAnalysisTree* aTree = (TRestAnalysisTree*)run->GetAnalysisTree();
    TRestRawSignalEvent* ev = (TRestRawSignalEvent*)run->GetInputEvent();

    if (run->GetEntries() != 50) {
        cout << "Number of entries is not 50!" << endl;
        return 1;
    }
    run->GetEntry(25);

    std::map<int, double> amps = aTree->GetObservableValue<map<int, double>>("vetoRaw_max_amplitude_map");

    Double_t average = 0;
    Int_t elems = 0;
    for (auto const& x : amps) {
        average += x.second;
        elems++;
    }
    average /= elems;

    if (elems != 14) {
        cout << "Number of veto signals identified is not 14! Computed value: " << elems << endl;
        return 2;
    }

    if ((Int_t)(average * 100.) != 4055) {
        cout << "The average amplitude (" << (Int_t)(average * 100.) << ") is wrong!" << endl;
        return 3;
    }

    std::vector<int> ids = ev->GetSignalIds();

    if (ids.size() != 20) {
        cout << "The number of detector signals is not 1!" << endl;
        return 4;
    }
    if (ids[10] != 4547) {
        cout << "Signal entry 10, id is not 4547!" << endl;
        return 5;
    }

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
