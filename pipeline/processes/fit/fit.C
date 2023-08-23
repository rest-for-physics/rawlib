
#include <TRestRawSignal.h>
#include <TRestRawSignalAddNoiseProcess.h>
#include <TRestRawSignalEvent.h>
#include <TRestRawSignalFittingProcess.h>
#include <TRestRawSignalShapingProcess.h>

using namespace std;

Int_t fit(Bool_t draw = false) {
    TRestRawSignalEvent* ev = new TRestRawSignalEvent();

    TRestRawSignal* signal = new TRestRawSignal();
    for (int n = 0; n < 512; n++) signal->AddPoint(0);
    signal->IncreaseBinBy(70, 100);
    ev->AddSignal(*signal);

    ///// Initializing processes through metadata definition
    string cfgFile = "metadata.rml";
    TRestRawSignalShapingProcess* shaper = new TRestRawSignalShapingProcess((char*)cfgFile.c_str());
    shaper->PrintMetadata();

    TRestRawSignalAddNoiseProcess* noise = new TRestRawSignalAddNoiseProcess((char*)cfgFile.c_str());
    noise->PrintMetadata();

    TRestRawSignalFittingProcess* fit = new TRestRawSignalFittingProcess((char*)cfgFile.c_str());
    fit->PrintMetadata();

    ///// Processing event
    TRestRawSignalEvent* shapedEvent = (TRestRawSignalEvent*)shaper->ProcessEvent(ev);

    TRestRawSignalEvent* noisyEvent = (TRestRawSignalEvent*)noise->ProcessEvent(shapedEvent);

    TRestRawSignalEvent* fittedEvent = (TRestRawSignalEvent*)fit->ProcessEvent(noisyEvent);

    cout << "Baseline: " << fit->GetBaseline() << endl;
    cout << "Amplitude: " << fit->GetAmplitude() << endl;
    cout << "Shaping: " << fit->GetShaping() << endl;
    cout << "Start position: " << fit->GetStartPosition() << endl;

    TF1* f = new TF1("fit1",
                     "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                     "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] * "
                     "sin((x-[3])/[2])/(1+TMath::Exp(-10000*(x-[3])))",
                     0, 511);
    f->SetParameters(fit->GetBaseline(), fit->GetAmplitude(), fit->GetShaping(), fit->GetStartPosition());

    if (draw) {
        TCanvas* c = new TCanvas();

        // Drawing the noisy event with fit result and original signal
        noisyEvent->DrawEvent();
        // shapedEvent->DrawEvent();

        f->SetLineColor(kOrange);
        f->SetLineWidth(6);
        f->Draw("same");

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
            h->Fill(i, singleSignal->GetRawData(i) - f->Eval(i));
        }
        h->Draw("hist");
    }

    Double_t sum = 0;
    for (int i = 0; i < shapedEvent->GetSignal(0)->GetNumberOfPoints(); i++) {
        sum += abs(shapedEvent->GetSignal(0)->GetRawData(i) - f->Eval(i));
    }

    cout << "Absolute value of difference original-fitted: " << sum << endl;

    if (sum > 15000) {
        cout << "Probably fitted fuction far from original." << endl;
        return 1;
    }

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
