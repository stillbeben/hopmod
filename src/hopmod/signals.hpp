#ifndef HOPMOD_SIGNALS_HPP
#define HOPMOD_SIGNALS_HPP

#include <boost/signal.hpp>

// Generic Server Events
extern boost::signal<void ()> signal_started;
extern boost::signal<void (int)> signal_shutdown;
extern boost::signal<void ()> signal_shutdown_scripting;
extern boost::signal<void ()> signal_reloadhopmod;
extern boost::signal<void ()> signal_maintenance;

#endif

