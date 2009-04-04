#ifndef HOPMOD_BANLIST_HPP
#define HOPMOD_BANLIST_HPP

class banned_networks;

bool write_banlist(const banned_networks *, const char *);
bool load_banlist(const char *, banned_networks *);

#endif
