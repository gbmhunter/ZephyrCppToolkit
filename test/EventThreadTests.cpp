#include <zephyr/ztest.h>

#include "EventThread.hpp"

namespace zct = zephyr_cpp_toolkit;

ZTEST_SUITE(EventThreadTests, NULL, NULL, NULL, NULL, NULL);

class Event {
    public:
        enum class Id {
            TIMER_EXPIRED,
            EXTERNAL_EVENT
        };

        Id id;
        int data;
};

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
                printk("Event received: %d\n", event.id);
            }
        }
};

void test_event_thread_create(void)
{
    MyEventThread eventThread;
    zassert_true(true, "Event thread created");
}