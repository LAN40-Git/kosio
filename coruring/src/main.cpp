#include "log/logger.h"

int main(int argc, const char * argv[]) {
    coruring::log::FileLogger logger("hello");
    logger.info("Hello, world!");
    return 0;
}
