
#ifndef HOPMOD_STOPWATCH_HPP
#define HOPMOD_STOPWATCH_HPP

class stopwatch
{
public:
    typedef unsigned long milliseconds;
    stopwatch();
    void start();
    stopwatch & stop();
    void resume();
    milliseconds get_elapsed()const;
    bool running()const;
private:
    unsigned long m_start;
    unsigned long m_stop;
};

#endif
