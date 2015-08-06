#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <functional>
#include <memory>
#include <ctime>

#include "MID.h"

struct Socket;
namespace msg {
    struct Message;
}

namespace msg {

    typedef std::function<void(Message*)> CallbackFunc;

    struct MIDCallback {

        MIDCallback(CMID m, CallbackFunc& f) : func(f), mid(m) {
            creation_time = time(&creation_time);
        }

        CallbackFunc func;
        CMID mid;
        time_t creation_time;
        float timeout_len = 5000000.0f;
    };

    typedef std::shared_ptr<MIDCallback> CallbackPtr;

    extern CallbackPtr make_MID_callback(CMID mid, CallbackFunc callback);
};

#endif