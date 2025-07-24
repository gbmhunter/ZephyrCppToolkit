/**************************************************************************************
 * @file IWatchdog.cpp
 * 
 * Implementation of the IWatchdog interface base class.
 *  
 **************************************************************************************
 * 
 * Copyright 2025 Electronic Timers Ltd.
 * 
 **************************************************************************************/

// 3rd party includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Local includes
#include "ZephyrCppToolkit/Peripherals/IWatchdog.hpp"

namespace zct {

LOG_MODULE_REGISTER(zct_IWatchdog, LOG_LEVEL_WRN);

IWatchdog::IWatchdog(const char* name) :
    m_name(name)
{
    __ASSERT_NO_MSG(name != nullptr);
    LOG_DBG("IWatchdog '%s' created.", m_name);
}

} // namespace zct