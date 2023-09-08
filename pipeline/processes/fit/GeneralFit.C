
#include <TRestRawSignal.h>
#include <TRestRawSignalAddNoiseProcess.h>
#include <TRestRawSignalEvent.h>
#include <TRestRawSignalFittingProcess.h>
#include <TRestRawSignalShapingProcess.h>

using namespace std;

Int_t GeneralFit(Bool_t draw = false) {
    TRestRawSignalEvent* ev = new TRestRawSignalEvent();

    TRestRawSignal* sgnl = new TRestRawSignal();
    for (int n = 0; n < 512; n++) {
        sgnl->AddPoint((Double_t)0);
    }
    sgnl->IncreaseBinBy(70, 100);
    ev->AddSignal(*sgnl);

    ///// Initializing processes through metadata definition
    string cfgFile = "metadata.rml";
    TRestRawSignalShapingProcess* shaper = new TRestRawSignalShapingProcess((char*)cfgFile.c_str());
    shaper->PrintMetadata();

    TRestRawSignalAddNoiseProcess* noise = new TRestRawSignalAddNoiseProcess((char*)cfgFile.c_str());
    noise->PrintMetadata();

    TRestRawSignalGeneralFitProcess* fit = new TRestRawSignalGeneralFitProcess((char*)cfgFile.c_str());
    fit->PrintMetadata();

    ///// Processing event
    TRestRawSignalEvent* shapedEvent = (TRestRawSignalEvent*)shaper->ProcessEvent(ev);

    TRestRawSignalEvent* noisyEvent = (TRestRawSignalEvent*)noise->ProcessEvent(shapedEvent);

    TRestRawSignalEvent* fittedEvent = (TRestRawSignalEvent*)fit->ProcessEvent(noisyEvent);

    if (draw) {
        TCanvas* c = new TCanvas();

        // Drawing the noisy event with fit result and original signal
        noisyEvent->DrawEvent();
        // shapedEvent->DrawEvent();

        fit->GetFunction()->SetLineColor(kOrange);
        fit->GetFunction()->SetLineWidth(6);
        fit->GetFunction()->Draw("same");

        TGraph* originalSignalGraph = shapedEvent->GetSignal(0)->GetGraph();
        originalSignalGraph->SetLineColor(kBlue);
        originalSignalGraph->SetLineWidth(4);
        originalSignalGraph->Draw("same");

        // Before noise
        TCanvas* c2 = new TCanvas();
        TRestRawSignal* singleSignal = shapedEvent->GetSignal(0);
        Int_t nBins = singleSignal->GetNumberOfPoints();
        TH1D* h = new TH1D("histo", "", nBins, 0, nBins);

        for (int i = 0; i < nBins; i++) {
            h->Fill(i, singleSignal->GetRawData(i) - fit->GetFunction()->Eval(i));
        }
        h->Draw("hist");
    }

    Double_t sum = 0;
    for (int i = 0; i < shapedEvent->GetSignal(0)->GetNumberOfPoints(); i++) {
        sum += abs(shapedEvent->GetSignal(0)->GetRawData(i) - fit->GetFunction()->Eval(i));
    }

    cout << "Absolute value of difference original-fitted: " << sum << endl;

    if (sum > 5000) {
        cout << "Probably fitted fuction far from original." << endl;
        return 1;
    }

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
