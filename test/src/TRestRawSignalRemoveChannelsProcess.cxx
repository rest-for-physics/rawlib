
#include <TRestRawSignalRemoveChannelsProcess.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace std;

const auto filesPath = fs::path(__FILE__).parent_path().parent_path() / "files";
const auto restRawSignalRemoveChannelsProcessRmlIds =
    filesPath / "TRestRawSignalRemoveChannelsProcessIds.rml";
const auto restRawSignalRemoveChannelsProcessRmlTypes =
    filesPath / "TRestRawSignalRemoveChannelsProcessTypes.rml";

TEST(TRestRawSignalRemoveChannelsProcess, TestFiles) {
    cout << "Test files path: " << filesPath << endl;

    // Check dir exists and is a directory
    EXPECT_TRUE(fs::is_directory(filesPath));
    // Check it's not empty
    EXPECT_TRUE(!fs::is_empty(filesPath));
    EXPECT_TRUE(fs::exists(restRawSignalRemoveChannelsProcessRmlIds));
    EXPECT_TRUE(fs::exists(restRawSignalRemoveChannelsProcessRmlTypes));
}

TEST(TRestRawSignalRemoveChannelsProcess, FromRmlIds) {
    TRestRawSignalRemoveChannelsProcess process(restRawSignalRemoveChannelsProcessRmlIds.c_str());

    process.PrintMetadata();

    const auto channelIds = process.GetChannelIds();

    std::vector<int> expectedChannelIds;
    for (int i = 4612; i <= 4888; i++) {
        expectedChannelIds.push_back(i);
    }

    EXPECT_EQ(channelIds, expectedChannelIds);

    const auto channelTypes = process.GetChannelTypes();
    EXPECT_EQ(channelTypes.size(), 0);
}

TEST(TRestRawSignalRemoveChannelsProcess, FromRmlTypes) {
    TRestRawSignalRemoveChannelsProcess process(restRawSignalRemoveChannelsProcessRmlTypes.c_str());

    process.PrintMetadata();

    const auto channelIds = process.GetChannelIds();
    EXPECT_EQ(channelIds.size(), 0);

    const auto channelTypes = process.GetChannelTypes();
    EXPECT_EQ(channelTypes.size(), 2);
    const set<string> expectedChannelTypes = {"tpc", "veto"};
    const set<string> channelTypesSet(channelTypes.begin(), channelTypes.end());
    EXPECT_EQ(channelTypesSet, expectedChannelTypes);
}
