#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ZephyrCppToolkit/Core/Mutex.hpp"

LOG_MODULE_REGISTER(MutexExample, LOG_LEVEL_DBG);

int main() {

   zct::Mutex mutex;
   auto lockGuard = mutex.lockGuard(K_MSEC(1000));
   __ASSERT_NO_MSG(lockGuard.didGetLock());

   LOG_INF("Mutex locked. It will be unlocked automatically when the lock guard goes out of scope.");

   return 0;
}