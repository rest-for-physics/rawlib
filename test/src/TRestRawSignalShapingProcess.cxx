
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
    TRestRawSignalShapingProcess rawSignalShapingProcess;
    EXPECT_TRUE(rawSignalShapingProcess.GetProcessName() == "rawSignalShaping");  // default name
}

TEST(TRestRawSignalShapingProcess, FromRml) {
    TRestRawSignalShapingProcess rawSignalShapingProcess((char*)restRawSignalShapingProcessRml.c_str());

    rawSignalShapingProcess.PrintMetadata();
}
