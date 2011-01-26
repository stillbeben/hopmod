#ifndef HOPMOD_SIGNALS_HPP
#define HOPMOD_SIGNALS_HPP

namespace fungu{namespace script{class env;}}
#include <boost/signal.hpp>
#include <limits>

/**
    @brief Event proceed combiner
    
    If any slot function returns -1 it will tell the signaller to veto/cancel the event.
*/
struct proceed
{
    typedef int result_type;
    static const int true_value = 0;
    template<typename InputIterator>
    int operator()(InputIterator first, InputIterator last)const
    {
        int cancel = 0;
        for(InputIterator it = first; it != last; ++it)
            if(*it == -1) cancel = -1;
        return cancel;
    }
};

struct maxvalue
{
    typedef int result_type;
    template<typename InputIterator>
    int operator()(InputIterator first, InputIterator last)const
    {
        if(first == last) return -1;
        int greatest = -1;
        for(InputIterator it = first; it != last; ++it) 
            greatest = std::max(greatest, *it);
        return greatest;
    }
};

// Generic Server Events
extern boost::signal<void ()> signal_started;
extern boost::signal<void (int)> signal_shutdown;
extern boost::signal<void ()> signal_shutdown_scripting;
extern boost::signal<void ()> signal_reloadhopmod;
extern boost::signal<void ()> signal_maintenance;

/**
    @brief Register signals with the global script::slot_factory instance.
*/
void register_signals(fungu::script::env &);

/**
    @brief Deallocates destroyed slots - should be called regularly on the main loop.
*/
void cleanup_dead_slots();

void disconnect_all_slots();

#endif
