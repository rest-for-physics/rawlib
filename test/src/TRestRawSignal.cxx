
#include <TRestRawSignal.h>
#include <gtest/gtest.h>

using namespace std;

TEST(TRestRawSignal, Default) {
    TRestRawSignal rawSignal;

    EXPECT_TRUE(rawSignal.GetIntegral() == 0);
}
