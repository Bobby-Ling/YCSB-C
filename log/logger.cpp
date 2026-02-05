#include "log.h"

__attribute__((constructor(100)))
inline static void global_logger_init() {
    INIT_LOGGER();
    LOG_INFO("inited");
}