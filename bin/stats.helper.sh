#!/bin/bash

###############################################################################
# conf
v_tmp=$(mktemp /tmp/hopmod_stats.XXXXXX)

f_statsdb=../log/stats.sqlite

c_sqlite="nice -n 9 sqlite3"
###############################################################################



###############################################################################
# clean unused v_tmp file, only random-name is needed
rm -f "$v_tmp"
###############################################################################



###############################################################################
# functions
###############################################################################
# rm users total stats
p_rmuser_stats() {

# $1 name
# "1" name doesnt exist

    l_tmp=
    l_tmp=$($c_sqlite "$f_statsdb" "SELECT * FROM playertotals WHERE name==\"$1\" ;")
    if [ -z "$l_tmp" ] ; then
	return 1
    else
	$($c_sqlite "$f_statsdb" "DELETE FROM players WHERE name==\"$1\" ;")
	$($c_sqlite "$f_statsdb" "DELETE FROM playertotals WHERE name==\"$1\" ;")
    fi
    return 0
}


# change users name in the total stats
p_chuser_name_stats() {

# $1 name | $2 newname
# "1" name doesnt exist
# "2" user-name: unnamed
# "3" user-name: bot
# "4" user-name longer 15 chars
# "5" newname exists

    if [ "$2" == "unnamed" ] ; then
	p_clean
	return 2
    fi
    if [ "$2" == "bot" ] ; then
	p_clean
	return 3
    fi
    if [ $(expr $(echo "$2" | wc -m) - 1) -gt 15 ] ; then
	p_clean
	return 4
    fi
    l_tmp=
    l_tmp=$($c_sqlite "$f_statsdb" "SELECT * FROM playertotals WHERE name==\"$1\" ;")
    if [ -z "$l_tmp" ] ; then
	return 1
    else
	l_tmp=
	l_tmp=$($c_sqlite "$f_statsdb" "SELECT * FROM playertotals WHERE name==\"$2\" ;")
	if [ -n "$l_tmp" ] ; then
	    return 5
	else
	    $($c_sqlite "$f_statsdb" "UPDATE players SET name=\"$2\" WHERE name==\"$1\" ;")
	    $($c_sqlite "$f_statsdb" "UPDATE playertotals SET name=\"$2\" WHERE name==\"$1\" ;")
	fi
    fi
    return 0
}


# list all users
p_list_users() {

    for a in $($c_sqlite "$f_statsdb" "SELECT name FROM playertotals ;") ; do
        echo "$a"
    done
}

p_user_in_stats_db() {

# $1 name
# returns:
# "0" name in
# "1" name not in

    l_tmp=
    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM playertotals WHERE name==\"$1\";")
    p_clean
    if [ -z "$l_tmp" ] ; then
        echo "1"
        return 1
    fi
    echo "0"
    return 0
}
###############################################################################
