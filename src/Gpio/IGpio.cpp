#include <zephyr/kernel.h>

#include "IGpio.hpp"

namespace zct {

IGpio::IGpio(const char* name, Direction direction, LogicMode logicMode) 
    : m_name(name), m_direction(direction), m_logicMode(logicMode)
{
    // Make sure the name is not null
    __ASSERT_NO_MSG(name != nullptr);
}

} // namespace zct