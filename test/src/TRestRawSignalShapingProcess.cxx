
#include <TRestRawSignalShapingProcess.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

const auto filesPath = fs::path(__FILE__).parent_path().parent_path() / "files";
const auto restRawSignalShapingProcessRml = filesPath / "TRestRawSignalShapingProcess.rml";

TEST(TRestRawSignalShapingProcess, TestFiles) {
    cout << "Test files path: " << filesPath << endl;

    // Check dir exists and is a directory
    EXPECT_TRUE(fs::is_directory(filesPath));
    // Check it's not empty
    EXPECT_TRUE(!fs::is_empty(filesPath));
    EXPECT_TRUE(fs::exists(restRawSignalShapingProcessRml));
}

TEST(TRestRawSignalShapingProcess, Default) {
    TRestRawSignalShapingProcess process;
    EXPECT_TRUE(process.GetProcessName() == "rawSignalShaping");

    EXPECT_TRUE(process.GetShapingType() == "shaperSin");
    EXPECT_TRUE(process.GetShapingTime() == 10.0);
    EXPECT_TRUE(process.GetShapingGain() == 1.0);
}

TEST(TRestRawSignalShapingProcess, FromRml) {
    TRestRawSignalShapingProcess process(restRawSignalShapingProcessRml.c_str());

    process.PrintMetadata();

    EXPECT_TRUE(process.GetShapingType() == "responseFile");
    EXPECT_TRUE(process.GetShapingTime() == 5.0);
    EXPECT_TRUE(process.GetShapingGain() == 20.0);
}
