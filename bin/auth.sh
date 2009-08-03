#!/bin/bash


###############################################################################
. $(pwd)/bin/auth.helper.sh
###############################################################################
#add domain: p_adddomain <domain>
#    errors:
#    "1" domain exists
#add domain on start: p_adddomain_onstart <domain>
#    errors: none
#add clan: p_addclan <name> <place> <tag[_front]> [<tag_back>]
#    errors:
#    "1" name exists
#    "2" place is not 0 or 1 or 2
#    "3" place is 2 but tag_back is missing
#    warning:
#    "4" tag[_front|_back] exists already
#add clan on start: p_addclan_onstart <name> <place> <tag[_front]> [<tag_back>]
#    errors: none
#add user: p_adduser <name> <domain> [<clan>]
#    errors:
#    "1" user-name: unnamed
#    "2" user-name: bot
#    "3" user-name longer 15 chars
#    "4" user-name exists
#    "5" domain doesnt exist
#    "6" clan doesnt exist
#add user on start: p_adduser_onstart <name> <domain> <pubkey> [<clan>]
#    errors: none
#delete user: p_rmuser <name> <domain>
#    errors:
#    "1" name doesnt exist
#    "2" domain doesnt exist
#delete clan: p_rmclan <clan>
#    errors:
#    "1" clan doesnt exist
#change users key: p_chuser_key <name> <domain>
#    errors:
#    "1" name doesnt exist
#    "2" domain doesnt exist
#change users clan: p_chuser_clan <name> <domain> <clan>
#    errors:
#    "1" name doesnt exist
#    "2" domain doesnt exist
#    "3" clan doesnt exist
#change users domain: p_chuser_domain <name> <domain> <new.domain>
#    errors:
#    "1" name doesnt exist
#    "2" domain doesnt exist
#    "3" newdomain doesnt exist
#    "4" name exists with newdomain
#change users name: p_chuser_name <name> <domain> <new.name>
#    errors:
#    "1" name doesnt exist
#    "2" domain doesnt exist
#    "3" user-name: unnamed
#    "4" user-name: bot
#    "4" user-name longer 15 chars
#    "5" newname exists
#rm domain: p_rmdomain <domain> [<fallback.domain>]
#    errors:
#    "1" domain doesnt exist
#    "2" newdomain doesnt exist
#    "3" a user exists with newdomain
#change domains name: p_chdomain_name <domain> <new.domain>
#    errors:
#    "1" domain doesnt exist
#    "2" newdomain exists
#fill auth.db with values of authdomainfile authtagfile authuserfile: p_firstrun
#    errors: none
#list all tags: p_list_clans
#    errors: none
#list all domains: p_list_domains
#    errors: none
#list all users [of a domain or all domains]: p_list_users [<domain>]
#    errors
#    "1" domain doesnt exist
#
#rm users stats: p_rmuser_stats <name>
#    errors:
#    "1" name doesnt exist
#change users name in the total stats: p_chuser_name_stats <name> <new.name>
#    errors:
#    "1" name doesnt exist
#    "2" user-name: unnamed
#    "3" user-name: bot
#    "4" user-name longer 15 chars
#    "5" newname exists
###############################################################################
# main
p_info() {
    echo "usage:"
    echo " "
    echo "user related functions:"
    echo "	$0 user add <name> <domain> [<clan>]"
    echo "	$0 user del <name> <domain>"
    echo "	$0 user ch  clan   <name> <domain> <clan>"
    echo "	$0 user ch  key    <name> <domain>"
    echo "	$0 user ch  name   <name> <domain> <new.name>"
    echo "	$0 user ch  domain <name> <domain> <new.domain>"
    echo " "
    echo "clan related functions:"
    echo "	$0 clan add <name> <place> <tag[_front]> [<tag_back>]"
    echo "	$0 clan del <name"
    echo " "
    echo "domain related functions:"
    echo "	$0 domain add <domain>"
    echo "	$0 domain del <domain> [<fallback.domain>]"
    echo "	$0 domain ch  <domain> <new.domain>"
    echo " "
    echo "listing functions"
    echo "	$0 list clans"
    echo "	$0 list domains"
    echo "	$0 list users [<domain>]"
    echo " "
    echo "others:"
    echo "	$0 firstrun"
    echo "		[writes the content of"
    echo "		 $f_authuserfile,"
    echo "		 $f_authtagfile and"
    echo "		 $f_authdomainfile"
    echo "		in a fresh auth.db]"
    echo "	$0 get default domain"
    echo " "
    exit 255
}
###############################################################################



if [ -z "$1" ] ; then
    p_info
fi

case $1 in
    user)
	case $2 in
	    add)
		if [ -z "$4" ] ; then
		    p_info
		fi
		if [ -z "$5" ] ; then
		    p_adduser "$3" "$4"
		else
		    p_adduser "$3" "$4" "$5"
		fi
	    ;;
	    del)
		if [ -z "$4" ] ; then
		    p_info
		fi
		p_deluser "$3" "$4"
	    ;;
	    ch)
		case $3 in
		    clan)
			if [ -z "$6" ] ; then
			    p_info
			fi
			p_chuser_clan "$4" "$5" "$6"
		    ;;
		    key)
			if [ -z "$5" ] ; then
			    p_info
			fi
			p_chuser_key "$4" "$5"
		    ;;
		    name)
			if [ -z "$6" ] ; then
			    p_info
			fi
			p_chuser_name "$4" "$5" "$6"
		    ;;
		    domain)
			if [ -z "$6" ] ; then
			    p_info
			fi
			p_chuser_domain "$4" "$5" "$6"
		    ;;
		    *)
			p_info
		    ;;
		esac
	    ;;
	    *)
		p_info
	    ;;
	esac
    ;;
    clan)
	case $2 in
	    add)
		if [ -z "$5" ] ; then
		    p_info
		fi
		v_place=
		if [ "$4" == "front" ] ; then
		    v_place="1"
		elif [ "$4" == "back" ] ; then
		    v_place="0"
		elif [ "$4" == "both" ] ; then
		    v_place="2"
		else
		    v_place="$4"
		fi
		if [ -z "$6" ] ; then
		    p_addclan "$3" "$v_place" "$5"
		else
		    p_addclan "$3" "$v_place" "$5" "$6"
		fi
	    ;;
	    del)
		if [ -z "$3" ] ; then
		    p_info
		fi
		p_delclan "$3"
	    ;;
	    *)
		p_info
	    ;;
	esac
    ;;
    domain)
	case $2 in
	    add)
		if [ -z "$3" ] ; then
		    p_info
		fi
		p_adddomain "$3"
	    ;;
	    del)
		if [ -z "$3" ] ; then
		    p_info
		fi
		if [ -z "$4" ] ; then
		    p_deldomain "$3"
		else
		    p_deldomain "$3" "$4"
		fi
	    ;;
	    ch)
		if [ -z "$4" ] ; then
		    p_info
		fi
		p_chdomain "$3" "$4"
	    ;;
	    *)
		p_info
	    ;;
	esac
    ;;
    list)
	case $2 in
	    clans)
		p_list_clans
	    ;;
	    domains)
		p_list_domains
	    ;;
	    users)
		if [ -z "$3" ] ; then
		    p_list_users
		else
		    p_list_users "$3"
		fi
	    ;;
	    *)
		p_info
	    ;;
	esac
    ;;
    firstrun)
	p_firstrun
    ;;
    get)
	case $2 in
	    default)
		case $3 in
		    domain)
			echo $v_default_domain
		    ;;
		    *)
			p_info
		    ;;
		esac
	    ;;
	    *)
		p_info
	    ;;
	esac
    ;;
    *)
	p_info
    ;;
esac

#exit $?
tmp=
tmp=$?
echo $tmp
exit $tmp
