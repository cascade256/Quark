#pragma once

extern "C" {
    bool initQuark(Global* globals);
    bool updateQuark(Global* globals);
    bool shutdownQuark(Global* globals);
}
