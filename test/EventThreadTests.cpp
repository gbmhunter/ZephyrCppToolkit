#include <variant>

#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

#include "EventThread.hpp"

namespace zct = zephyr_cpp_toolkit;

LOG_MODULE_REGISTER(EventThreadTests, LOG_LEVEL_DBG);

ZTEST_SUITE(EventThreadTests, NULL, NULL, NULL, NULL, NULL);

// class Event {
//     public:
//         enum class Id {
//             TIMER_EXPIRED,
//             EXTERNAL_EVENT
//         };

//         Id id;
//         int data;
// };


struct MyTimerTimeoutEvent {
};

struct ExitEvent {};

struct LedFlashingEvent {
    uint32_t flashRateMs;
};

typedef std::variant<MyTimerTimeoutEvent, LedFlashingEvent, ExitEvent> Event;

class MyEventThread : public zct::EventThread<Event> {
    public:
        MyEventThread() :
            zct::EventThread<Event>(threadStack, THREAD_STACK_SIZE, eventQueueBuffer, EVENT_QUEUE_NUM_ITEMS)
        {
        }

    private:
        static constexpr size_t EVENT_QUEUE_NUM_ITEMS = 10;
        static constexpr size_t THREAD_STACK_SIZE = 512;
        K_KERNEL_STACK_MEMBER(threadStack, THREAD_STACK_SIZE);
        Event eventQueueBuffer[EVENT_QUEUE_NUM_ITEMS];

        void threadMain() override {
            while (1) {
                Event event = zct::EventThread<Event>::waitForEvent();
                LOG_DBG("Event received: %d.", event.index());
                if (std::holds_alternative<MyTimerTimeoutEvent>(event)) {
                    LOG_DBG("Got MyTimerTimeoutEvent.");
                } else if (std::holds_alternative<LedFlashingEvent>(event)) {
                    LOG_DBG("Got LedFlashingEvent.");
                } else if (std::holds_alternative<ExitEvent>(event)) {
                    LOG_DBG("Got ExitEvent.");
                    break;
                }
            }
        }
};

ZTEST(EventThreadTests, testEventThreadCreate)
{
    MyEventThread eventThread;
    zassert_true(true, "Event thread created");

    // Send a LED on event
    LedFlashingEvent ledFlashingEvent = { .flashRateMs = 1000 }; // Create an Event lvalue directly
    eventThread.sendEvent(ledFlashingEvent);

    k_sleep(K_SECONDS(1));

    // Send an exit event
    ExitEvent exitEvent;
    eventThread.sendEvent(exitEvent);

    k_sleep(K_SECONDS(1));

}