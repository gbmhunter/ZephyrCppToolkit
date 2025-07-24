#include <zephyr/kernel.h>

#include "ZephyrCppToolkit/Peripherals/IAdc.hpp"

namespace zct {

IAdc::IAdc(const char* name) :
    m_name(name)
{
    // Make sure the name is not null
    __ASSERT_NO_MSG(name != nullptr);
}

IAdc::~IAdc() {}

} // namespace zct