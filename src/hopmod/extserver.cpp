#ifdef INCLUDE_EXTSERVER_CPP

void player_msg(int cn,const char * text)
{
    get_ci(cn)->sendprivtext(text);
}

#endif
