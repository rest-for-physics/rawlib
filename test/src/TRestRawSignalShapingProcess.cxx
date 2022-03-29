
#include <TRestRawSignalShapingProcess.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

#define FILES_PATH fs::path(__FILE__).parent_path().parent_path() / "files"
const auto TRestRawSignalShapingProcessRml = FILES_PATH / "TRestRawSignalShapingProcess.rml";

TEST(TRestRawSignalShapingProcess, TestFiles) {
    cout << "Test files path: " << FILES_PATH << endl;

    // Check dir exists and is a directory
    EXPECT_TRUE(fs::is_directory(FILES_PATH));
    // Check it's not empty
    EXPECT_TRUE(!fs::is_empty(FILES_PATH));
    EXPECT_TRUE(fs::exists(TRestRawSignalShapingProcessRml));
}

TEST(TRestRawSignalShapingProcess, Default) {
    TRestRawSignalShapingProcess rawSignalShapingProcess;
    EXPECT_TRUE(rawSignalShapingProcess.GetProcessName() == "rawSignalShaping");  // default name
}

TEST(TRestRawSignalShapingProcess, FromRml) {
    TRestRawSignalShapingProcess rawSignalShapingProcess((char*)TRestRawSignalShapingProcessRml.c_str());

    rawSignalShapingProcess.PrintMetadata();
}
