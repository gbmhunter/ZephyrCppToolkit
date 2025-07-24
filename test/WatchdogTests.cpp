#include <zephyr/ztest.h>

#include "ZephyrCppToolkit/Peripherals/WatchdogMock.hpp"
#include "ZephyrCppToolkit/Peripherals/WatchdogReal.hpp"

ZTEST_SUITE(WatchdogTests, NULL, NULL, NULL, NULL, NULL);

// Note: Device tree spec for real watchdog tests (if hardware available)
// static const struct device* l_testWdtDevice = DEVICE_DT_GET(DT_ALIAS(watchdog0));

ZTEST(WatchdogTests, mockWatchdogCanBeConfigured)
{
    zct::WatchdogMock wdt("TestWdt");
    
    // Should not be setup initially
    zassert_false(wdt.mockIsSetup(), "Watchdog should not be setup initially");
    zassert_false(wdt.mockIsDisabled(), "Watchdog should not be disabled initially");
    zassert_equal(wdt.mockGetChannelCount(), 0, "Should have no channels initially");
    
    // Install a timeout
    int channelId = wdt.installTimeout(1000);
    zassert_true(channelId >= 0, "Channel installation should succeed");
    zassert_equal(wdt.mockGetChannelCount(), 1, "Should have one channel after installation");
    
    // Setup watchdog
    int result = wdt.setup();
    zassert_equal(result, 0, "Setup should succeed");
    zassert_true(wdt.mockIsSetup(), "Watchdog should be setup");
}

ZTEST(WatchdogTests, mockWatchdogChannelInfo)
{
    zct::WatchdogMock wdt("TestWdt");
    
    // Install timeout with specific parameters
    uint32_t timeoutMs = 2000;
    bool callbackTriggered = false;
    auto callback = [&callbackTriggered](int channelId, void* userData) {
        callbackTriggered = true;
    };
    
    int channelId = wdt.installTimeout(timeoutMs, callback, nullptr, 
                                     zct::IWatchdog::ResetFlag::ResetCpuCore);
    
    // Check channel info
    const auto* info = wdt.mockGetChannelInfo(channelId);
    zassert_not_null(info, "Channel info should exist");
    zassert_equal(info->timeoutMs, timeoutMs, "Timeout should match");
    zassert_equal(info->flags, zct::IWatchdog::ResetFlag::ResetCpuCore, "Flags should match");
    zassert_false(info->isActive, "Channel should not be active before setup");
    
    // Setup and check activation
    wdt.setup();
    info = wdt.mockGetChannelInfo(channelId);
    zassert_true(info->isActive, "Channel should be active after setup");
}

ZTEST(WatchdogTests, mockWatchdogFeedingWorks)
{
    zct::WatchdogMock wdt("TestWdt");
    
    int channelId = wdt.installTimeout(1000);
    wdt.setup();
    
    // Initial feed count should be 0
    zassert_equal(wdt.mockGetFeedCount(channelId), 0, "Initial feed count should be 0");
    
    // Feed the watchdog
    int result = wdt.feed(channelId);
    zassert_equal(result, 0, "Feed should succeed");
    zassert_equal(wdt.mockGetFeedCount(channelId), 1, "Feed count should increment");
    
    // Feed again
    result = wdt.feed(channelId);
    zassert_equal(result, 0, "Second feed should succeed");
    zassert_equal(wdt.mockGetFeedCount(channelId), 2, "Feed count should increment again");
}

ZTEST(WatchdogTests, mockWatchdogTimeoutCallback)
{
    zct::WatchdogMock wdt("TestWdt");
    
    bool callbackTriggered = false;
    int callbackChannelId = -1;
    void* callbackUserData = nullptr;
    
    auto callback = [&](int channelId, void* userData) {
        callbackTriggered = true;
        callbackChannelId = channelId;
        callbackUserData = userData;
    };
    
    int testData = 42;
    int channelId = wdt.installTimeout(1000, callback, &testData);
    wdt.setup();
    
    // Manually trigger timeout
    wdt.mockTriggerTimeout(channelId);
    
    zassert_true(callbackTriggered, "Callback should be triggered");
    zassert_equal(callbackChannelId, channelId, "Callback should receive correct channel ID");
    zassert_equal(callbackUserData, &testData, "Callback should receive correct user data");
}

ZTEST(WatchdogTests, mockWatchdogTimeRemaining)
{
    zct::WatchdogMock wdt("TestWdt");
    
    int channelId = wdt.installTimeout(1000);
    wdt.setup();
    
    // Check time remaining (should be close to 1000ms)
    int64_t timeRemaining = wdt.mockGetTimeRemainingMs(channelId);
    zassert_true(timeRemaining > 900 && timeRemaining <= 1000, 
                "Time remaining should be close to timeout value");
    
    // Feed the watchdog and check again
    wdt.feed(channelId);
    timeRemaining = wdt.mockGetTimeRemainingMs(channelId);
    zassert_true(timeRemaining > 900 && timeRemaining <= 1000, 
                "Time remaining should reset after feeding");
}

ZTEST(WatchdogTests, mockWatchdogDisable)
{
    zct::WatchdogMock wdt("TestWdt");
    
    int channelId = wdt.installTimeout(1000);
    wdt.setup();
    
    // Disable watchdog
    int result = wdt.disable();
    zassert_equal(result, 0, "Disable should succeed");
    zassert_true(wdt.mockIsDisabled(), "Watchdog should be disabled");
    zassert_false(wdt.mockIsSetup(), "Watchdog should not be setup after disable");
    
    // Feeding should fail after disable
    result = wdt.feed(channelId);
    zassert_not_equal(result, 0, "Feed should fail on disabled watchdog");
}

ZTEST(WatchdogTests, mockWatchdogErrorHandling)
{
    zct::WatchdogMock wdt("TestWdt");
    
    // Try to feed before setup
    int result = wdt.feed(0);
    zassert_not_equal(result, 0, "Feed should fail before setup");
    
    // Try to feed invalid channel
    wdt.setup();
    result = wdt.feed(999);
    zassert_not_equal(result, 0, "Feed should fail for invalid channel");
    
    // Try operations after disable
    wdt.disable();
    result = wdt.installTimeout(1000);
    zassert_not_equal(result, 0, "Install timeout should fail on disabled watchdog");
    
    result = wdt.setup();
    zassert_not_equal(result, 0, "Setup should fail on disabled watchdog");
}

ZTEST(WatchdogTests, mockWatchdogReset)
{
    zct::WatchdogMock wdt("TestWdt");
    
    // Setup some state
    wdt.installTimeout(1000);
    wdt.setup();
    
    zassert_true(wdt.mockIsSetup(), "Should be setup");
    zassert_equal(wdt.mockGetChannelCount(), 1, "Should have one channel");
    
    // Reset and verify clean state
    wdt.mockReset();
    
    zassert_false(wdt.mockIsSetup(), "Should not be setup after reset");
    zassert_false(wdt.mockIsDisabled(), "Should not be disabled after reset");
    zassert_equal(wdt.mockGetChannelCount(), 0, "Should have no channels after reset");
}

ZTEST(WatchdogTests, mockWatchdogMultipleChannels)
{
    zct::WatchdogMock wdt("TestWdt");
    
    // Install multiple timeouts
    int channel1 = wdt.installTimeout(1000);
    int channel2 = wdt.installTimeout(2000);
    int channel3 = wdt.installTimeout(500);
    
    zassert_equal(wdt.mockGetChannelCount(), 3, "Should have three channels");
    
    wdt.setup();
    
    // Feed different channels
    wdt.feed(channel1);
    wdt.feed(channel2);
    wdt.feed(channel2);  // Feed channel2 twice
    
    zassert_equal(wdt.mockGetFeedCount(channel1), 1, "Channel1 should be fed once");
    zassert_equal(wdt.mockGetFeedCount(channel2), 2, "Channel2 should be fed twice");
    zassert_equal(wdt.mockGetFeedCount(channel3), 0, "Channel3 should not be fed");
    
    // Check different timeout values
    const auto* info1 = wdt.mockGetChannelInfo(channel1);
    const auto* info2 = wdt.mockGetChannelInfo(channel2);
    const auto* info3 = wdt.mockGetChannelInfo(channel3);
    
    zassert_equal(info1->timeoutMs, 1000, "Channel1 timeout should be 1000ms");
    zassert_equal(info2->timeoutMs, 2000, "Channel2 timeout should be 2000ms");
    zassert_equal(info3->timeoutMs, 500, "Channel3 timeout should be 500ms");
}

// Uncomment this test if you have real watchdog hardware available and properly configured
/*
ZTEST(WatchdogTests, realWatchdogBasicOperation)
{
    if (!device_is_ready(l_testWdtDevice)) {
        ztest_test_skip();
        return;
    }
    
    zct::WatchdogReal wdt("RealWdt", l_testWdtDevice);
    
    // Install timeout
    int channelId = wdt.installTimeout(5000);  // 5 second timeout
    zassert_true(channelId >= 0, "Channel installation should succeed");
    
    // Setup watchdog
    int result = wdt.setup();
    zassert_equal(result, 0, "Setup should succeed");
    
    // Feed the watchdog
    result = wdt.feed(channelId);
    zassert_equal(result, 0, "Feed should succeed");
    
    // Disable watchdog
    result = wdt.disable();
    zassert_equal(result, 0, "Disable should succeed");
}
*/