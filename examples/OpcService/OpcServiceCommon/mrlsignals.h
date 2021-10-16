#ifndef MRLSIGNALS_H
#define MRLSIGNALS_H

#include <vector>
#include <map>
#include <string>
//
#include <Wt/WSignal>

//
// Convience macros to declare signals
//
#define SIGNAL0(n)         \
private:                   \
    Wt::Signal<void> _##n; \
                           \
public:                    \
    Wt::Signal<void>& n() { return _##n; }

#define SIGNAL1(n, t)   \
private:                \
    Wt::Signal<t> _##n; \
                        \
public:                 \
    Wt::Signal<t>& n() { return _##n; }

#define SIGNALT(n, t) \
private:              \
    T _##n;           \
                      \
public:               \
    T& n() { return _##n; }

#endif  // MRLSIGNALS_H
