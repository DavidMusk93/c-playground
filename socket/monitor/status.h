#ifndef MONITOR_STATUS_H
#define MONITOR_STATUS_H

enum Code {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kConflict = 6,
};

#endif //MONITOR_STATUS_H
