//
// Created by esgyn on 1/19/2021.
//

#ifndef MONITOR_LOOP_H
#define MONITOR_LOOP_H

#include "sock.h"

namespace sun {
    class Loop : public nocopy {
    public:
        io::Poll &pollInstance() {
            return poll_;
        }

        virtual void loop() {
            poll_.loop();
        }

    private:
        io::Poll poll_;
    };
}

#endif //MONITOR_LOOP_H
