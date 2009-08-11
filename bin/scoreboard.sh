#!/bin/bash



###############################################################################
v_tmp=$(mktemp /tmp/hopmod_scoreboard.XXXXXX)

f_statsdb=./log/stats.sqlite

f_authhelper=./bin/auth.helper.sh


d_target=./share/web/scoreboard.html

c_sqlite="nice -n 19 sqlite3"
c_cp="nice -n 9 cp"
###############################################################################



###############################################################################
f_statsdb_tmp=$v_tmp
mkdir -p $d_target
# clean unused v_tmp file, only random-name is needed
rm -f "$v_tmp"
###############################################################################



###############################################################################
p_info() {
    echo "usage:"
    echo "	$0 [auth]"
    echo " "
    exit 255
}

$c_cp "$f_statsdb" "$f_statsdb_tmp".sqlite

auth_used=0
if [ -n "$1" ] && [ "$1" == "auth" ] ; then
    auth_used=1
fi

if [ "$auth_used" == "1" ] ; then
    . $f_authhelper
fi

cur_date=$(date "+%m/%d/%y %r")

cat > "$d_target"/total.html << EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<body text="#000000">
    <br>
    <center>
	<font face="verdana" size=1>last updated: $cur_date</font>
    </center>
    <br>
    <br>
    <table align="center" border="1">
	<tr>
	    <td colspan="7">
		<i>total&nbsp;scoreboard</i>
	    </td>
	</tr>
	<tr>
	    <td width="8%" vAlign=top noWrap>
		<font size=1>rank</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>name</font>
	    </td>
	    <td width="15%" vAlign=top noWrap>
	    	<font size=1>frags</font>
	    </td>
	    <td width="15%" vAlign=top noWrap>
	    	<font size=1>deaths</font>
	    </td>
	    <td width="15%" vAlign=top noWrap>
		<font size=1>kills&nbsp;per&nbsp;death</font>
	    </td>
	    <td width="12%" vAlign=top noWrap>
		<font size=1>accuracy</font>
	    </td>
	    <td width="12%" vAlign=top noWrap>
		<font size=1>games</font>
	    </td>
	</tr>
EOF
rank=1
v_players=
if [ "$auth_used" == "1" ] ; then
    v_players="SELECT id FROM playertotals ORDER BY frags DESC,deaths ASC,max_frags DESC,teamkills ASC,suicides ASC, hits DESC, shots ASC, games ASC, timeplayed ASC, wins DESC, losses ASC ;"
else
    v_players="SELECT id FROM playertotals WHERE name!=\"unnamed\" AND name!=\"bot\" ORDER BY frags DESC,deaths ASC,max_frags DESC,teamkills ASC,suicides ASC, hits DESC, shots ASC, games ASC, timeplayed ASC, wins DESC, losses ASC ;"
fi
for a in $($c_sqlite "$f_statsdb_tmp".sqlite "$v_players") ; do
    name=
    co=
    last_game=
    frags=
    maxfrags=
    deaths=
    suis=
    tks=
    hits=
    shots=
    damage=
    damagewasted=
    wins=
    losses=
    games=
    draws=
    time=
    name=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT name FROM playertotals WHERE id=\"$a\" ;")
    co=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT country FROM playertotals WHERE id=\"$a\" ;")
    last_game=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT last_game FROM playertotals WHERE id=\"$a\" ;")
    frags=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT frags FROM playertotals WHERE id=\"$a\" ;")
    maxfrags=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT max_frags FROM playertotals WHERE id=\"$a\" ;")
    deaths=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT deaths FROM playertotals WHERE id=\"$a\" ;")
    suis=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT suicides FROM playertotals WHERE id=\"$a\" ;")
    tks=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT teamkills FROM playertotals WHERE id=\"$a\" ;")
    hits=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT hits FROM playertotals WHERE id=\"$a\" ;")
    shots=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT shots FROM playertotals WHERE id=\"$a\" ;")
    damage=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT damage FROM playertotals WHERE id=\"$a\" ;")
    damagewasted=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT damagewasted FROM playertotals WHERE id=\"$a\" ;")
    wins=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT wins FROM playertotals WHERE id=\"$a\" ;")
    losses=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT losses FROM playertotals WHERE id=\"$a\" ;")
    games=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT games FROM playertotals WHERE id=\"$a\" ;")
    draws=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT withdraws FROM playertotals WHERE id=\"$a\" ;")
    time=$($c_sqlite "$f_statsdb_tmp".sqlite "SELECT timeplayed FROM playertotals WHERE id=\"$a\" ;")
    clan=
    if [ "$auth_used" == "1" ] ; then
	clan=$(p_get_clan_name "$name" "$v_default_domain")
	if [ "$?" == "0" ] ; then
	    tag=
	    tag=$(p_get_clan_tag "front" "$clan")
	    if [ "$?" == "0" ] ; then
		name="$tag$name"
	    fi
	    tag=
	    tag=$(p_get_clan_tag "back" "$clan")
	    if [ "$?" == "0" ] ; then
		name="$name$tag"
	    fi
	else
	    clan="none"
	fi
    fi
    last_game_was=$(date -d "1970-01-01 $last_game sec utc" "+%m/%d/%y %r")
    l_time_d=
    l_time_d_r=
    l_time_h=
    l_time_h_r=
    l_time_m=
    l_time_s=
    if [ "$(expr $time \>\= 86400)" == "1" ] ; then
        l_time_d=$(expr $time / 86400)
        l_time_d_r=$(expr $time - $(expr $l_time_d \* 86400))
        if ! [ "$l_time_d_r" == "0" ] ; then
	    l_time_h=$(expr $l_time_d_r / 3600)
	    l_time_h_r=$(expr $l_time_d_r - $(expr $l_time_h \* 3600))
	    l_time_h_r=$(expr $l_time_d_r % 3600)
	    if ! [ "$l_time_h_r" == "0" ] ; then
	        l_time_m=$(expr $l_time_h_r / 60)
	        l_time_s=$(expr $l_time_h_r - $(expr $l_time_m \* 60))
	    fi
	fi
    elif [ "$(expr $time \>\= 3600)" == "1" ] ; then
        l_time_h=$(expr $time / 3600)
        l_time_h_r=$(expr $time - $(expr $l_time_h \* 3600))
        if ! [ "$l_time_h_r" == "0" ] ; then
	    l_time_m=$(expr $l_time_h_r / 60)
	    l_time_s=$(expr $l_time_h_r - $(expr $l_time_m \* 60))
        fi
    elif [ "$(expr $time \>\= 60)" == "1" ] ; then
        l_time_m=$(expr $time / 60)
        l_time_s=$(expr $time - $(expr $l_time_m \* 60))
    else
        l_time_s=$time
    fi
    time_formated=
    time_formated="${l_time_d:=0}d ${l_time_h:="0"}h ${l_time_m:="0"}m ${l_time_s:="0"}s"
    acc=0.00
    l_hits=
    l_hits=$(expr $hits \* 10000)
    if ! [ "$shots" == "0" ] ; then
        l_acc=$(expr $l_hits / $shots)
        l_acc_length=$(expr length $l_acc)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_acc 1 $(expr $l_acc_length - 2))
        l_t2=$(expr substr $l_acc $(expr $l_acc_length - 1) $l_acc_length)
        acc=${l_t1:-"0"}.${l_t2:-"00"}
    fi
    kpd=0.00
    l_frags=
    l_frags=$(expr $frags \* 100)
    if ! [ "$deaths" == "0" ] ; then
        l_kpd=$(expr $l_frags / $deaths)
        l_kpd_length=$(expr length $l_kpd)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_kpd 1 $(expr $l_kpd_length - 2))
        l_t2=$(expr substr $l_kpd $(expr $l_kpd_length - 1) $l_kpd_length)
        kpd=${l_t1:-"0"}.${l_t2:-"00"}
    fi
    fpg=0.00
    dpg=0.00
    tpg=0.00
    spg=0.00
    l_deaths=$(expr $deaths \* 100)
    l_tks=$(expr $tks \* 100)
    l_suis=$(expr $suis \* 100)
    if ! [ "$games" == "0" ] ; then
        l_fpg=$(expr $l_frags / $games)
        l_fpg_length=$(expr length $l_fpg)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_fpg 1 $(expr $l_fpg_length - 2))
        l_t2=$(expr substr $l_fpg $(expr $l_fpg_length - 1) $l_fpg_length)
        fpg=${l_t1:-"0"}.${l_t2:-"00"}
        l_dpg=$(expr $l_deaths / $games)
        l_dpg_length=$(expr length $l_dpg)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_dpg 1 $(expr $l_dpg_length - 2))
        l_t2=$(expr substr $l_dpg $(expr $l_dpg_length - 1) $l_dpg_length)
        dpg=${l_t1:-"0"}.${l_t2:-"00"}
        l_tpg=$(expr $l_tks / $games)
        l_tpg_length=$(expr length $l_tpg)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_tpg 1 $(expr $l_tpg_length - 2))
        l_t2=$(expr substr $l_tpg $(expr $l_tpg_length - 1) $l_tpg_length)
        tpg=${l_t1:-"0"}.${l_t2:-"00"}
        l_spg=$(expr $l_suis / $games)
        l_spg_length=$(expr length $l_spg)
        l_t1=
        l_t2=
        l_t1=$(expr substr $l_spg 1 $(expr $l_spg_length - 2))
        l_t2=$(expr substr $l_spg $(expr $l_spg_length - 1) $l_spg_length)
        spg=${l_t1:-"0"}.${l_t2:-"00"}
    fi
    damagetotal=$(expr $damage + $damagewasted)
    cat >> "$d_target"/total.html << EOF
	<tr>
	    <td width="8%" vAlign=top noWrap>
		<font size=1>$rank</font>
	    </td>
	    <td vAlign=top noWrap>
		<a href="$a.html"><font size=1>$name</font></a>
	    </td>
	    <td width="15%" vAlign=top noWrap>
		<font size=1>$frags</font>
	    </td>
	    <td width="15%" vAlign=top noWrap>
		<font size=1>$deaths</font>
	    </td>
	    <td width="15%" vAlign=top noWrap>
		<font size=1>$kpd</font>
	    </td>
	    <td width="12%" vAlign=top noWrap>
		<font size=1>$acc%</font>
	    </td>
	    <td width="12%" vAlign=top noWrap>
		<font size=1>$games</font>
	    </td>
	</tr>
EOF
	cat > "$d_target"/$a.html << EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<body text="#000000">
    <table align="center" border="1">
	<tr>
	    <td colspan="2">
		<i>player's&nbsp;scoreboard</i>
	    </td>
	</tr>
	<tr>
	    <td vAlign=top noWrap colspan="2">
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;$name
	    </td>
	</tr>
EOF
	if [ "$auth_used" == "1" ] ; then
	    cat >> "$d_target"/$a.html << EOF
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>clan:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$clan</font>
	    </td>
	</tr>
EOF
	fi
	cat >> "$d_target"/$a.html << EOF
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>country:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$co</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>rank:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$rank</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>frags:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$frags</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>deaths:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$deaths</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>teamkills:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$tks</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>suicides:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$suis</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>max&nbsp;frags&nbsp;in&nbsp;a&nbsp;game:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$maxfrags</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>kills&nbsp;per&nbsp;death:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$kpd</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>accuracy:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$acc%</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>frags&nbsp;per&nbsp;game:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$fpg</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>deaths&nbsp;per&nbsp;game:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$dpg</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>teamkills&nbsp;per&nbsp;game:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$tpg</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>suicids&nbsp;per&nbsp;game:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$spg</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>hits/shots:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$hits/$shots</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>damage/total damage:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$damage/$damagetotal</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>games:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$games</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>wins:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$wins</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>losses:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$losses</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>draws:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$draws</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>played&nbsp;time:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$time_formated</font>
	    </td>
	</tr>
	<tr>
	    <td width="20%" vAlign=top noWrap>
		<font size=1>last&nbsp;game&nbsp;was&nbsp;at:</font>
	    </td>
	    <td vAlign=top noWrap>
		<font size=1>$last_game_was</font>
	    </td>
	</tr>
    </table>
    <br>
    <br>
    <br>
    <center>
	<font face="verdana" size=1>last updated: $cur_date</font>
    </center>
    <br>
</body>
</html>
EOF
    rank=$(expr $rank + 1)
done
    
cat >> "$d_target"/total.html << EOF
    </table>
    <br>
</body>
</html>
EOF

rm -f "$f_statsdb_tmp".sqlite

exit 0
