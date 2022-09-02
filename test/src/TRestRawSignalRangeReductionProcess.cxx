
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
    EXPECT_TRUE(process.GetProcessName() == "rawSignalRangeReductionProcess");

    EXPECT_TRUE(process.GetResolutionInNumberOfBits() == 12);
    EXPECT_TRUE(process.GetDigitizationInputRange() ==
                TVector2(std::numeric_limits<Short_t>::min(), std::numeric_limits<Short_t>::max()));
}

TEST(TRestRawSignalRangeReductionProcess, FromRml) {
    TRestRawSignalRangeReductionProcess process(restRawSignalShapingProcessRml.c_str());

    process.PrintMetadata();

    EXPECT_TRUE(process.GetResolutionInNumberOfBits() == 10);
    EXPECT_TRUE(process.GetDigitizationInputRange() == TVector2(-1000, 10000));
}
