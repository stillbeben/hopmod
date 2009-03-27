#ifndef HOPMOD_SCRIPT_PIPE_HPP
#define HOPMOD_SCRIPT_PIPE_HPP

#include "env_fwd.hpp"

void init_script_pipe();
bool open_script_pipe(const char *,int,fungu::script::env &);
void run_script_pipe_service(int);
void close_script_pipe();

#endif
