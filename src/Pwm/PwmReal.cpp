#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "PwmReal.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_PwmReal, LOG_LEVEL_DBG);

PwmReal::PwmReal(const char* name) 
    : IPwm(name)
{}

} // namespace zct