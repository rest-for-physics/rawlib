
#include <TRestRawSignalRangeReductionProcess.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

const auto filesPath = fs::path(__FILE__).parent_path().parent_path() / "files";
const auto restRawSignalShapingProcessRml = filesPath / "TRestRawSignalRangeReductionProcess.rml";

TEST(TRestRawSignalRangeReductionProcess, TestFiles) {
    cout << "Test files path: " << filesPath << endl;

    // Check dir exists and is a directory
    EXPECT_TRUE(fs::is_directory(filesPath));
    // Check it's not empty
    EXPECT_TRUE(!fs::is_empty(filesPath));
    EXPECT_TRUE(fs::exists(restRawSignalShapingProcessRml));
}

TEST(TRestRawSignalRangeReductionProcess, Default) {
    TRestRawSignalRangeReductionProcess process;
    EXPECT_TRUE(process.GetProcessName() == (std::string) "rawSignalRangeReductionProcess");

    EXPECT_TRUE(process.GetResolutionInNumberOfBits() == 12);

    EXPECT_TRUE(process.GetDigitizationInputRange().X() == numeric_limits<Short_t>::min());
    EXPECT_TRUE(process.GetDigitizationInputRange().Y() == numeric_limits<Short_t>::max());
}

TEST(TRestRawSignalRangeReductionProcess, FromRml) {
    TRestRawSignalRangeReductionProcess process(restRawSignalShapingProcessRml.c_str());

    process.PrintMetadata();

    EXPECT_TRUE(process.GetResolutionInNumberOfBits() == 10);

    EXPECT_TRUE(process.GetDigitizationInputRange().X() == -1000);
    EXPECT_TRUE(process.GetDigitizationInputRange().Y() == 10000);
}

TEST(TRestRawSignalRangeReductionProcess, Event) {
    TRestRawSignalRangeReductionProcess process;
    process.SetResolutionInNumberOfBits(12);
    process.SetDigitizationInputRange({-2000, 12000});

    TRestRawSignalEvent event;
    TRestRawSignal signal;
    for (int i = 0; i < 512; i++) {
        if (i == 100) {
            signal.AddPoint(process.GetDigitizationInputRange().Y());
        } else {
            signal.AddPoint(process.GetDigitizationInputRange().X());
        }
    }
    event.AddSignal(signal);

    process.Initialize();
    process.InitProcess();
    const auto output = (TRestRawSignalEvent*)process.ProcessEvent(&event);

    const auto outputSignal = output->GetSignal(0);

    output->PrintEvent();
    EXPECT_TRUE(outputSignal->GetMinValue() == 0);
    EXPECT_TRUE(outputSignal->GetMaxValue() == 4095);
}
