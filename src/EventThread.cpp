#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "EventThread.hpp"

LOG_MODULE_REGISTER(EventThread, LOG_LEVEL_DBG);

namespace zephyr_cpp_toolkit {


constexpr uint32_t MAX_EVENT_SIZE_BYTES = 32;


} // namespace zephyr_cpp_toolkit
