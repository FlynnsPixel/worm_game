#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <sstream>
#include "PlatformConfig.h"

#define ERR_NONE 0
#define ERR_UDP_PING_PONG_FAILED 9000

#define SSTR(x) dynamic_cast<std::ostringstream&>((std::ostringstream() << std::dec << x)).str()

namespace err {

    extern int get_last_error();
    extern int fresult;
};

#endif