#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"
#include "lib/handle_resolver.hpp"

boost::signal<void (int)> signal_shutdown;
boost::signal<void ()> signal_shutdown_scripting;
boost::signal<void ()> signal_reloadhopmod;
boost::signal<void ()> signal_maintenance;

void disconnect_all_slots()
{
    signal_shutdown.disconnect_all_slots();
    signal_shutdown_scripting.disconnect_all_slots();
    signal_reloadhopmod.disconnect_all_slots();
    signal_maintenance.disconnect_all_slots();
}

