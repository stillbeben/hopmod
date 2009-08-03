#!/bin/bash


###############################################################################
. $(pwd)/bin/stats.helper.sh
###############################################################################
#list all users: p_list_users
#    errors
#    "1" domain doesnt exist
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
    echo "	$0 user del <name>"
    echo "	$0 user ch  name   <name> <new.name>"
    echo " "
    echo "listing functions"
    echo "	$0 list users"
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
	    del)
		if [ -z "$3" ] ; then
		    p_info
		fi
		p_rmuser_stats "$3"
	    ;;
	    ch)
		case $3 in
		    name)
			if [ -z "$5" ] ; then
			    p_info
			fi
			p_chuser_name_stats "$4" "$5"
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
    list)
	case $2 in
	    users)
		p_list_users
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
