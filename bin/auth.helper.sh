#!/bin/bash

###############################################################################
# conf
v_tmp=$(mktemp /tmp/hopmod_auth.XXXXXX)

v_default_domain=

d_authserver=$(pwd)

f_authdb=$d_authserver/log/auth.sqlite
f_authserveruserfile=$d_authserver/bin/authserver.users
f_authuserfile=$d_authserver/bin/auth.users
f_authtagfile=$d_authserver/bin/auth.tags
f_authdomainfile=$d_authserver/bin/auth.domains

f_scripthelper=$d_authserver/bin/auth.helper.sh

c_sqlite="nice -n 9 sqlite3"
###############################################################################



###############################################################################
# clean unused v_tmp file, only random-name is needed
rm -f "$v_tmp"
###############################################################################



###############################################################################
# functions
###############################################################################
p_restartserver() {
    (
        cd "$d_authserver"
        bin/authserver stop
        bin/authserver
    )
    return 0
}

# wrote p_* output
p_return() {

# $1 content | $2 fileend
# returns $v_tmp.$2 with content $1

    echo "$1" > "$v_tmp"."$2"
    return 0
}


# clean p_* outputs
p_clean() {
    for a in domainid pubkey privkey tagid ; do
	if [ -f "$v_tmp"."$a" ] ; then
	    rm -f "$v_tmp"."$a"
	fi
    done
    return 0
}


# get domainid
p_getdomainid() {

# $1 domain
# "1" domain doesnt exist
# returns domainid in $v_tmp.domainid

    v_domainid=
    v_domainid=$($c_sqlite "$f_authdb" "SELECT id FROM domains WHERE name==\"$1\";")
    if [ -z "$v_domainid" ]; then
	return 1
    fi
    p_return "$v_domainid" "domainid"
    return 0
}


# generate keypair
p_genkeypair() {

# returns pubkey in $v_tmp.pubkey | privkey in $v_tmp.privkey"

    (
	cd "$d_authserver/bin"
	l_stepone=1
	for a in $(./keygen) ; do
	    if [ "$l_stepone" == "1" ]; then
		p_return "$a" "privkey"
		l_stepone=0
	    else
		p_return "$a" "pubkey"
		break
	    fi
	done
    )
    return 0
}


# get clantagid
p_gettagid() {

# $1 clan
# "1" clan doesnt exist
# returns tagid in $v_tmp.tagid

    v_tagid=
    v_tagid=$($c_sqlite "$f_authdb" "SELECT id FROM tags WHERE name==\"$1\";")
    if [ -z "$v_tagid" ]; then
	return 1
    fi
    p_return "$v_tagid" "tagid"
    return 0
}


# add user
p_adduser() {

# $1 name | $2 domain | [$3 clan]
# "1" user-name: unnamed
# "2" user-name: bot
# "3" user-name longer 15 chars
# "4" user-name exists
# "5" domain doesnt exist
# "6" clan doesnt exist

    if [ "$1" == "unnamed" ] ; then
        p_clean
	return 1
    fi
    if [ "$1" == "bot" ] ; then
        p_clean
	return 2
    fi
    if [ $(expr $(echo "$1" | wc -m) - 1) -gt 15 ] ; then
        p_clean
	return 3
    fi
    if ! $(p_getdomainid "$2") ; then
        p_clean
	return 5
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
	if [ -n "$l_tmp" ]; then
	    p_clean
	    return 4
	else
	    p_genkeypair
	    v_pubkey=
	    v_pubkey=$(cat "$v_tmp".pubkey)
	    v_privkey=
	    v_privkey=$(cat "$v_tmp".privkey)
	    if [ -z "$3" ]; then
		$($c_sqlite "$f_authdb" "INSERT INTO users (domain_id,name,pubkey) VALUES ($v_domainid,\"$1\",\"$v_pubkey\") ;")
		echo "p_adduser_onstart \"$1\" \"$2\" \"$v_pubkey\"" >> "$f_authuserfile"
	    else
		if ! $(p_gettagid "$3") ; then
		    p_clean
		    return 6
		else
		    v_tagid=
		    v_tagid=$(cat "$v_tmp".tagid)
		    $($c_sqlite "$f_authdb" "INSERT INTO users (domain_id,name,pubkey,clan_id) VALUES ($v_domainid,\"$1\",\"$v_pubkey\","$v_tagid") ;")
		    echo "p_adduser_onstart \"$1\" \"$2\" \"$v_pubkey\" \"$3\"" >> "$f_authuserfile"
		fi
	    fi
	    echo "adduser \"$1\" \"$2\" \"$v_pubkey\"" >> "$f_authserveruserfile"
	    echo "adduser \"$1\" \"$2\" \"$v_pubkey\"" > "$d_authserver/authexec"
	fi
    fi
echo "authkey \"$1\" \"$v_privkey\" \"$2\""
    p_clean
    return 0
}


# add user on start
p_adduser_onstart() {

# $1 name | $2 domain | $3 pubkey | [$4 clan]

    p_getdomainid "$2"
    v_domainid=
    v_domainid=$(cat "$v_tmp".domainid)
    l_tmp=
    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\" ;")
    if [ -z "$l_tmp" ] ; then
        if [ -z "$4" ] ; then
	    $($c_sqlite "$f_authdb" "INSERT INTO users (domain_id,name,pubkey) VALUES ($v_domainid,\"$1\",\"$3\") ;")
	else
	    p_gettagid "$4"
	    v_tagid=$(cat "$v_tmp".tagid)
	    $($c_sqlite "$f_authdb" "INSERT INTO users (domain_id,name,pubkey,clan_id) VALUES ($v_domainid,\"$1\",\"$3\",$v_tagid) ;")
	fi
    fi
    p_clean
    return 0
}


# add clan
p_addclan() {

# $1 name | $2 place | $3 tag[_front] | [$4 tag_back]
# "1" name exists
# "2" place is not 0 or 1 or 2
# "3" place is 2 but tag_back is missing
# Warning: "4" tag[_front|_back] exists already

    l_tmp=
    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE name==\"$1\" ;")
    if [ -n "$l_tmp" ]; then
	p_clean
	return 1
    fi
    if ! ( [ "$2" == "0" ] || [ "$2" == "1" ] || [ "$2" == "2" ] ) ; then
	p_clean
	return 2
    elif [ "$2" == "2" ] && [ "$4" == "" ] ; then
	p_clean
	return 3
    fi
    warn=0
    if [ "$2" == "0" ] ; then
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE tag_back==\"$3\" ;")
	if [ -n "$l_tmp" ] ; then
	    warn=1
	fi
	$($c_sqlite "$f_authdb" "INSERT INTO tags (name,place,tag_back) VALUES (\"$1\",0,\"$3\") ;")
	echo "p_addclan_onstart \"$1\" \"0\" \"$3\"" >> "$f_authtagfile"
    elif [ "$2" == "1" ] ; then
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE tag_front==\"$3\" ;")
	if [ -n "$l_tmp" ] ; then
	    warn=1
	fi
	$($c_sqlite "$f_authdb" "INSERT INTO tags (name,place,tag_front) VALUES (\"$1\",1,\"$3\") ;")
	echo "p_addclan_onstart \"$1\" \"1\" \"$3\"" >> "$f_authtagfile"
    else
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE tag_front==\"$3\" ;")
	if [ -n "$l_tmp" ] ; then
	    warn=1
	else
	    l_tmp=
	    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE tag_back==\"$4\" ;")
	    if [ -n "$l_tmp" ] ; then
		warn=1
	    fi
	fi
	$($c_sqlite "$f_authdb" "INSERT INTO tags (name,place,tag_front,tag_back) VALUES (\"$1\",2,\"$3\",\"$4\") ;")
	echo "p_addclan_onstart \"$1\" \"2\" \"$3\" \"$4\"" >> "$f_authtagfile"
    fi
    p_clean
    if [ "$warn" == "1" ] ; then
	return 4
    fi
    return 0
}


# add clan on start
p_addclan_onstart() {

# $1 name | $2 place | $3 tag[_front] | [$4 tag_back]

    l_tmp=
    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM tags WHERE name==\"$1\";")
    if [ -z "$l_tmp" ]; then
	v_do="INSERT"
	if [ "$2" == "0" ] ; then
	    $($c_sqlite "$f_authdb" "$v_do INTO tags (name,place,tag_back) VALUES (\"$1\",0,\"$3\") ;")
	elif [ "$2" == "1" ] ; then
	    $($c_sqlite "$f_authdb" "$v_do INTO tags (name,place,tag_front) VALUES (\"$1\",1,\"$3\") ;")
	else
	    $($c_sqlite "$f_authdb" "$v_do INTO tags (name,place,tag_front,tag_back) VALUES (\"$1\",2,\"$3\",\"$4\") ;")
	fi
    fi
    p_clean
    return 0
}


# delete user
p_rmuser() {

# $1 name | $2 domain
# "1" name doesnt exist
# "2" domain doesnt exist

    if ! $(p_getdomainid "$2") ; then
        p_clean
        return 2
    else
	v_domainid=
        v_domainid=$(cat "$v_tmp".domainid)
    	l_tmp=
        l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
        if [ -z "$l_tmp" ]; then
            p_clean
            return 1
        else
            $($c_sqlite "$f_authdb" "DELETE FROM users WHERE domain_id==\"$v_domainid\" AND name==\"$1\";")
            for a in "$f_authuserfile" "$f_authserveruserfile" ; do
        	cat "$a" | grep -v -E "\"$1\" \w*\"$2\"" > "$v_tmp".auf
        	mv "$v_tmp".auf "$a"
    	    done
            echo "deleteuser \"$1\" \"$2\"" > "$d_authserver/authexec"
        fi
    fi
    p_clean
    return 0
}


# delete clan
p_rmclan() {

# $1 clan
# "1" clan doesnt exist

    if ! $(p_gettagid "$1") ; then
	p_clean
	return 1
    else
	v_tagid=
	v_tagid=$(cat "$v_tmp".tagid)
	$($c_sqlite "$f_authdb" "UPDATE users SET clan_id=NULL WHERE clan_id==\"$v_tagid\" ;")
	$($c_sqlite "$f_authdb" "DELETE FROM tags WHERE id==\"$v_tagid\" ;")
	cat "$f_authuserfile" | sed "s/\ \"$(echo "$1" | sed 's/ /\\\ /g')\"//" > "$v_tmp".auf
	mv "$v_tmp".auf "$f_authuserfile"
	cat "$f_authtagfile" | grep -v "$1" > "$v_tmp".auf
	mv "$v_tmp".auf "$f_authtagfile"
    fi
    p_clean
    return 0
}


# change users key
p_chuser_key() {

# $1 name | $2 domain
# "1" name doesnt exist
# "2" domain doesnt exist

    if ! $(p_getdomainid "$2") ; then
        p_clean
        return 2
    else
	v_domainid=
        v_domainid=$(cat "$v_tmp".domainid)
        l_tmp=
        l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
        if [ -z "$l_tmp" ]; then
            p_clean
            return 1
        else
    	    p_genkeypair
    	    v_pubkey=
	    v_pubkey=$(cat "$v_tmp".pubkey)
	    v_privkey=
	    v_privkey=$(cat "$v_tmp".privkey)
	    $($c_sqlite "$f_authdb" "UPDATE users SET pubkey=\"$v_pubkey\" WHERE domain_id==\"$v_domainid\" AND name==\"$1\";")
    	    v_tagid=
    	    v_tagid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
    	    if [ -n "$v_tagid" ]; then
    		v_clantag=
    		v_clantag=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_tagid\" ;")
    	    fi
    	    for a in "$f_authuserfile" "$f_authserveruserfile" ; do
    		cat "$a" | grep -v -E "\"$1\" \w*\"$2\"" > "$v_tmp".auf
    		mv "$v_tmp".auf "$a"
    	    done
    	    echo "deleteuser \"$1\" \"$2\"" > "$d_authserver/authexec"
            if [ -z "$v_tagid" ] ; then
        	echo "p_adduser_onstart \"$1\" \"$2\" \"$v_pubkey\"" >> "$f_authuserfile"
            else
            	echo "p_adduser_onstart \"$1\" \"$2\" \"$v_pubkey\" \"$v_clantag\"" >> "$f_authuserfile"
            fi
            echo "adduser \"$1\" \"$2\" \"$v_pubkey\"" >> "$f_authserveruserfile"
            echo "adduser \"$1\" \"$2\" \"$v_pubkey\"" > "$d_authserver/authexec"
        fi
    fi
echo "authkey \"$1\" \"$v_privkey\" \"$2\""
    p_clean
    return 0
}


# change users tag
p_chuser_clan() {

# $1 name | $2 domain | $3 clan
# "1" name doesnt exist
# "2" domain doesnt exist
# "3" clan doesnt exist

    if ! $(p_getdomainid "$2") ; then
        p_clean
        return 2
    else
	v_domainid=
        v_domainid=$(cat "$v_tmp".domainid)
        l_tmp=
        l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
        if [ -z "$l_tmp" ]; then
            p_clean
            return 1
        elif ! $(p_gettagid "$3") ; then
	    p_clean
	    return 3
	else
	    v_tagid=
	    v_tagid=$(cat "$v_tmp".tagid)
	    $($c_sqlite "$f_authdb" "UPDATE users SET clan_id=\"$v_tagid\" WHERE domain_id==\"$v_domainid\" AND name==\"$1\" ;")
    	    cat "$f_authuserfile" | grep -v -E "\"$1\" \w*\"$2\"" > "$v_tmp".auf
    	    mv "$v_tmp".auf "$f_authuserfile"
    	    v_pubkey=
    	    v_pubkey=$($c_sqlite "$f_authdb" "SELECT pubkey FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\" AND clan_id==\"$v_tagid\";")
	    echo "p_adduser_onstart \"$1\" \"$2\" \"$v_pubkey\" \"$3\"" >> "$f_authuserfile"
        fi
    fi
    p_clean
    return 0
}


# change users domain
p_chuser_domain() {

# $1 name | $2 domain | $3 newdomain
# "1" name doesnt exist
# "2" domain doesnt exist
# "3" newdomain doesnt exist
# "4" name exists with newdomain

    if ! $(p_getdomainid "$2") ; then
        p_clean
        return 2
    else
	v_domainid=
        v_domainid=$(cat "$v_tmp".domainid)
        l_tmp=
        l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
        if [ -z "$l_tmp" ]; then
            p_clean
            return 1
        elif ! $(p_getdomainid "$3") ; then
	    p_clean
	    return 3
	else
	    v_newdomainid=
	    v_newdomainid=$(cat "$v_tmp".domainid)
	    l_tmp=
	    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_newdomainid\";")
	    if [ -n "$l_tmp" ] ; then
		p_clean
		return 4
	    else
		$($c_sqlite "$f_authdb" "UPDATE users SET domain_id=\"$v_newdomainid\" WHERE domain_id==\"$v_domainid\" AND name==\"$1\" ;")
		for a in "$f_authuserfile" "$f_authserveruserfile" ; do
    		    cat "$a" | grep -v -E "\"$1\" \w*\"$2\"" > "$v_tmp".auf
    		    mv "$v_tmp".auf "$a"
    		done
    		echo "deleteuser \"$1\" \"$2\"" > "$d_authserver/authexec"
    		v_pubkey=
    		v_pubkey=$($c_sqlite "$f_authdb" "SELECT pubkey FROM users WHERE name==\"$1\" AND domain_id==\"$v_newdomainid\" ;")
    		v_tagid=
    		v_tagid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE name==\"$1\" AND domain_id==\"$v_newdomainid\" ;")
    		if [ -n "$v_tagid" ] ; then
    		    v_clan=
		    v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_tagid\" ;")
		    echo "p_adduser_onstart \"$1\" \"$3\" \"$v_pubkey\" \"$v_clan\"" >> "$f_authuserfile"
		else
		    echo "p_adduser_onstart \"$1\" \"$3\" \"$v_pubkey\"" >> "$f_authuserfile"
        	fi
    		echo "adduser \"$1\" \"$3\" \"$v_pubkey\"" >> "$f_authserveruserfile"
        	echo "adduser \"$1\" \"$3\" \"$v_pubkey\"" > "$d_authserver/authexec"
    	    fi
	fi
    fi
    p_clean
    return 0
}


# change users name
p_chuser_name() {

# $1 name | $2 domain | $3 newname
# "1" name doesnt exist
# "2" domain doesnt exist
# "3" user-name: unnamed
# "4" user-name: bot
# "5" user-name longer 15 chars
# "6" newname exists

    if [ "$3" == "unnamed" ] ; then
	p_clean
	return 3
    fi
    if [ "$3" == "bot" ] ; then
	p_clean
	return 4
    fi
    if [ $(expr $(echo "$3" | wc -m) - 1) -gt 15 ] ; then
	p_clean
	return 5
    fi
    if ! $(p_getdomainid "$2") ; then
        p_clean
        return 2
    else
	v_domainid=
        v_domainid=$(cat "$v_tmp".domainid)
        l_tmp=
        l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$1\" AND domain_id==\"$v_domainid\";")
        if [ -z "$l_tmp" ]; then
            p_clean
            return 1
        else
    	    l_tmp=
    	    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE name==\"$3\" AND domain_id==\"$v_domainid\";")
    	    if [ -n "$l_tmp" ]; then
        	p_clean
        	return 6
    	    else
    	        $($c_sqlite "$f_authdb" "UPDATE users SET name=\"$3\" WHERE domain_id==\"$v_domainid\" AND name==\"$1\";")
    	    fi
	    for a in "$f_authuserfile" "$f_authserveruserfile" ; do
		cat "$a" | grep -v -E "\"$1\" \w*\"$2\"" > "$v_tmp".auf
		mv "$v_tmp".auf "$a"
	    done
    	    echo "deleteuser \"$1\" \"$2\"" > "$d_authserver/authexec"
    	    v_pubkey=
    	    v_pubkey=$($c_sqlite "$f_authdb" "SELECT pubkey FROM users WHERE name==\"$3\" AND domain_id==\"$v_domainid\" ;")
    	    v_tagid=
    	    v_tagid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE name==\"$3\" AND domain_id==\"$v_domainid\" ;")
    	    if [ -n "$v_tagid" ] ; then
    		v_clan=
		v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_tagid\" ;")
		echo "p_adduser_onstart \"$3\" \"$2\" \"$v_pubkey\" \"$v_clan\"" >> "$f_authuserfile"
	    else
		echo "p_adduser_onstart \"$3\" \"$2\" \"$v_pubkey\"" >> "$f_authuserfile"
            fi
            echo "adduser \"$3\" \"$2\" \"$v_pubkey\"" >> "$f_authserveruserfile"
            echo "adduser \"$3\" \"$2\" \"$v_pubkey\"" > "$d_authserver/authexec"
    	fi
    fi
    p_clean
    return 0
}


# add domain
p_adddomain() {

# $1 domain
# "1" domain exists

    if $(p_getdomainid "$1") ; then
        p_clean
        return 1
    else
        $($c_sqlite "$f_authdb" "INSERT INTO domains (name) VALUES (\"$1\") ;")
        echo "p_adddomain_onstart \"$1\"" >> "$f_authdomainfile"
    fi
    p_clean
    return 0
}


# add domain on start
p_adddomain_onstart() {

# $1 domain

    if ! $(p_getdomainid "$1") ; then
        $($c_sqlite "$f_authdb" "INSERT INTO domains (name) VALUES (\"$1\") ;")
    fi
    p_clean
    return 0
}


# rm domain
p_rmdomain() {

# $1 domain | [ $2 fallback domain ]
# "1" domain doesnt exist
# "2" newdomain doesnt exist
# "3" a user exists with newdomain

    if ! $(p_getdomainid "$1") ; then
	p_clean
	return 1
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	if [ -z "$2" ] ; then
	    $($c_sqlite "$f_authdb" "DELETE FROM users WHERE domain_id==\"$v_domainid\" ;")
	    for a in "$f_authuserfile" "$f_authserveruserfile" ; do
		cat "$a" | grep -v "\ \"$1\"" > "$v_tmp".auf
		mv "$v_tmp".auf "$a"
	    done
	else
	    if ! $(p_getdomainid "$2") ; then
		p_clean
		return 2
	    else
		v_newdomainid=
		v_newdomainid=$(cat "$v_tmp".domainid)
		for a in $($c_sqlite "$f_authdb" "SELECT name FROM users WHERE domain_id==\"$v_domainid\" ;") ; do
		    l_tmp=
		    l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE domain_id==\"$v_newdomainid\" AND name==\"$a\";")
		    if [ -n "$l_tmp" ] ; then
			p_clean
			return 3
		    fi
		done
		$($c_sqlite "$f_authdb" "UPDATE users SET domain_id=\"$v_newdomainid\" WHERE domain_id==\"$v_domainid\" ;")
		for a in "$f_authuserfile" "$f_authserveruserfile" ; do
		    cat "$a" | sed "s/\ \"$1\"\ /\ \"$2\"\ /" > "$v_tmp".auf
		    mv "$v_tmp".auf "$a"
		done
	    fi
	fi
	$($c_sqlite "$f_authdb" "DELETE FROM domains WHERE id==\"$v_domainid\" AND name==\"$1\" ;")
	cat "$f_authdomainfile" | grep -v "\ \"$1\"" > "$v_tmp".auf
	mv "$v_tmp".auf "$f_authdomainfile"
	p_restartserver
    fi
    p_clean
    return 0
}


# change domains name
p_chdomain_name() {

# $1 domain | $2 newdomain
# "1" domain doesnt exist
# "2" newdomain exists

    if ! $(p_getdomainid "$1") ; then
	p_clean
	return 1
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	if $(p_getdomainid "$2") ; then
	    p_clean
	    return 2
	else
	    $($c_sqlite "$f_authdb" "UPDATE domains SET name=\"$2\" WHERE id==\"$v_domainid\" AND name==\"$1\" ;")
	    for a in "$f_authuserfile" "$f_authserveruserfile" "$f_authdomainfile" ; do
		cat "$a" | sed "s/\ \"$1\"/\ \"$2\"/" > "$v_tmp".auf
		mv "$v_tmp".auf "$a"
	    done
	    p_restartserver
	fi
    fi
    p_clean
    return 0
}


# rm users total stats
p_rmuser_stats() {

# $1 name
# "1" name doesnt exist

    l_tmp=
    l_tmp=$($c_sqlite "$f_statsdb" "SELECT * FROM playertotals WHERE name==\"$1\" ;")
    if [ -z "$l_tmp" ] ; then
	p_clean
	return 1
    else
	$($c_sqlite "$f_statsdb" "DELETE FROM playertotals WHERE name==\"$1\" ;")
    fi
    p_clean
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
	p_clean
	return 1
    else
	l_tmp=
	l_tmp=$($c_sqlite "$f_statsdb" "SELECT * FROM playertotals WHERE name==\"$2\" ;")
	if [ -n "$l_tmp" ] ; then
	    p_clean
	    return 5
	else
	    $($c_sqlite "$f_statsdb" "UPDATE playertotals SET name=\"$2\" WHERE name==\"$1\" ;")
	fi
    fi
    p_clean
    return 0
}


# fill auth.db with values of authdomainfile authtagfile authuserfile
p_firstrun() {
    cat > "$v_tmp".firstrun << EOF
#!/bin/bash

. "$f_scripthelper"

EOF
    for a in "$f_authdomainfile" "$f_authtagfile" "$f_authuserfile" ; do
	cat "$a" >> "$v_tmp.firstrun"
    done
    bash "$v_tmp".firstrun
    rm -f "$v_tmp".firstrun
    p_clean
    return 0
}


# list all clans
p_list_clans() {
    for a in $($c_sqlite "$f_authdb" "SELECT id FROM tags ;") ; do
	v_tagplace=
	v_tagplace=$($c_sqlite "$f_authdb" "SELECT place FROM tags WHERE id==\"$a\" ;")
	v_clan=
	v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$a\" ;")
	v_tag=
	if [ "$v_tagplace" == "0" ] ; then
	    v_tag=$($c_sqlite "$f_authdb" "SELECT tag_back FROM tags WHERE id==\"$a\" ;")
	    echo "$v_clan     $v_tag"
	elif [ "$v_tagplace" == "1" ] ; then
	    v_tag=$($c_sqlite "$f_authdb" "SELECT tag_front FROM tags WHERE id==\"$a\" ;")
	    echo "$v_tag     $v_clan"
	else
	    v_tag=$($c_sqlite "$f_authdb" "SELECT tag_front FROM tags WHERE id==\"$a\" ;")
	    v_tag2=
	    v_tag2=$($c_sqlite "$f_authdb" "SELECT tag_back FROM tags WHERE id==\"$a\" ;")
	    echo "$v_tag     $v_clan     $v_tag2"
	fi
    done
    p_clean
    return 0
}


# list all domains
p_list_domains() {
    echo $($c_sqlite "$f_authdb" "SELECT name FROM domains ;")
    p_clean
    return 0
}

# list all users
p_list_users() {

# [$1 domain]
# "1" domain doesnt exist

    if [ -z "$1" ] ; then
	for a in $($c_sqlite "$f_authdb" "SELECT id FROM domains ;") ; do
	    echo "$($c_sqlite "$f_authdb" "SELECT name FROM domains WHERE id==\"$a\" ;")"
	    for b in $($c_sqlite "$f_authdb" "SELECT name FROM users WHERE domain_id==\"$a\" ;") ; do
		v_clanid=
		v_clanid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE name==\"$b\" AND domain_id==\"$a\" ;")
		if [ -n "$v_clanid" ] ; then
		    v_clan=
		    v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_clanid\" ;")
		    echo "$b   $v_clan"
		else
		    echo "$b"
		fi
	    done
	    echo " "
	done
    elif ! $(p_getdomainid "$1") ; then
	p_clean
	return 1
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	for a in $($c_sqlite "$f_authdb" "SELECT name FROM users WHERE domain_id==\"$v_domainid\" ;") ; do
	    v_clanid=
	    v_clanid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE name==\"$a\" AND domain_id==\"$v_domainid\" ;")
	    if [ -n "$v_clanid" ] ; then
	        v_clan=
	        v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_clanid\" ;")
	        echo "$a   $v_clan"
	    else
	        echo "$a"
	    fi
	done
    fi
    p_clean
    return 0
}

p_user_in_auth_db() {

# $1 name | $2 domain
# returns:
# "0" name in
# "1" name not in
# errors:
# "2" domain doesnt exists

    if ! $(p_getdomainid "$2") ; then
	p_clean
	return 2
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE domain_id==\"$v_domainid\" AND name==\"$1\";")
	p_clean
	if [ -z "$l_tmp" ] ; then
	    echo "1"
	    return 1
	fi
	echo "0"
	return 0
    fi
}

p_get_clan_name() {

# $1 name | $2 domain
# returns:
# "0" and on std-out: clan
# "1" not in clan
# errors:
# "2" name doesnt exists
# "3" domain doesnt exists

    if ! $(p_getdomainid "$2") ; then
	p_clean
	return 3
    else
	v_domainid=
	v_domainid=$(cat "$v_tmp".domainid)
	l_tmp=
	l_tmp=$($c_sqlite "$f_authdb" "SELECT * FROM users WHERE domain_id==\"$v_domainid\" AND name==\"$1\" ;")
	if [ -z "$l_tmp" ] ; then
	    p_clean
	    return 2
	else
	    v_clanid=
	    v_clanid=$($c_sqlite "$f_authdb" "SELECT clan_id FROM users WHERE domain_id==\"$v_domainid\" AND name==\"$1\" ;")
	    if [ -z "$v_clanid" ] ; then
		p_clean
		return 1
	    else
		v_clan=
		v_clan=$($c_sqlite "$f_authdb" "SELECT name FROM tags WHERE id==\"$v_clanid\" ;")
		echo "$v_clan"
	    fi
	fi
    fi
    p_clean
    return 0
}

p_get_clan_tag() {

# $1 front|back | $2 clan
# returns:
# "0" and on std-out: tag_[front|back]
# "1" tag doesnt exists
# errors:
# "2" $1 not front or back
# "3" clan doesnt exists

    if ! ( [ "$1" == "front" ] || [ "$1" == "back" ] ) ; then
	p_clean
	return 2
    else
	v_clanid=
	v_clanid=$($c_sqlite "$f_authdb" "SELECT id FROM tags WHERE name==\"$2\" ;")
	if [ -z "$v_clanid" ] ; then
	    p_clean
	    return 3
	else
	    v_tag=
	    if [ "$1" == "front" ] ; then
		v_tag=$($c_sqlite "$f_authdb" "SELECT tag_front FROM tags WHERE id==\"$v_clanid\" ;")
	    else
		v_tag=$($c_sqlite "$f_authdb" "SELECT tag_back FROM tags WHERE id==\"$v_clanid\" ;")
	    fi
	    if [ -z "$v_tag" ] ; then
	        p_clean
	        return 1
	    fi
	    echo "$v_tag"
	fi
    fi
    p_clean
    return 0
}
###############################################################################
