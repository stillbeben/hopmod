playercmd_login = [
        local loginpassword ""
        local loginip ""
        local firstlogin ""
        local ipaddr ""
	local greet ""
	local adminlvl ""
	if (= (player_pvar $cn logged_in) 1) [privmsg $cn (red[You are already logged in])][
		statsdb eval [select password from register where name=$arg1] [loginpassword = (column password)
		if (strcmp $arg2 "") [privmsg $cn (format "%1" (red [Nice try!]) )] [
			if (strcmp $loginpassword $arg2) [
         	 		statsdb eval [select firstlogin from register where name=$arg1] [firstlogin = (column firstlogin)]
         	 		statsdb eval [select ipaddr from register where name=$arg1] [loginip = (column ipaddr)]
        	 		statsdb eval [select greet from register where name=$arg1] [greet = (column greet)]
	         		statsdb eval [select admin from register where name =$arg1] [adminlvl = (column admin)]
	     			if (= $firstlogin 1) [
                        		playerip = (player_ip $cn)
                        		privmsg $cn (format "%1" (blue [This is your first login!]) )
                        		statsdb eval [update register set firstlogin = 0 where name = $arg1]
                        		statsdb eval [update register set ipaddr = $playerip where name = $arg1]
                		]
				if (= $adminlvl 1) [player_pvar $cn adminlvl 1]
                		player_pvar $cn logged_in 1
				player_pvar $cn logged_in_as $arg1
                		msg (format "%1 %2 %3" (blue (player_name $cn)) (orange [is now logged in as]) (red (player_pvar $cn logged_in_as)) )
                		privmsg $cn (format "Last login from IP: %1" (green $loginip) )
                		privmsg $cn (format "Your IP now: %1" (green (player_ip $cn)) )
				if (= (player_pvar $cn adminlvl) 1) [
					privmsg $cn (format "Type: %1 to get master/admin." (red [#getmaster]) )
				]
				privmsg $cn (format "Type %1 to see your commandlist" (red [#cmds]) )
                		playerip = (player_ip $cn)
                		statsdb eval [update register set ipaddr = $playerip where name = $arg1]
                		player_pvar $cn mute 0
                		if (= (player_pvar $cn speccr) 1) [unspec $cn]
				if (!(strcmp $greet "0")) [msg (format "Greet Message: %1 - %2" (green $arg1) (green $greet) ) ]
			] [ privmsg $cn (format "%1" (red [Wrong username or password!]) ) ]

		]
	]
]

playercmd_getmaster = [
	if (&& (= (player_pvar $cn adminlvl) 1) (= (player_pvar $cn logged_in) 1)) [
		setpriv $cn admin
		privmsg $cn (format "Type: %1 to leave master/admin." (red [#leavemaster]) )
	] [privmsg $cn (red [Permission Denied]) ]
]

playercmd_leavemaster = [
        if (= (player_pvar $cn logged_in) 1) [
                setpriv $cn none
		privmsg $cn (format "Type: %1 to get master/admin." (red [#getmaster]) )
        ] [privmsg $cn (red [Permission Denied]) ]
]

playercmd_logout = [
        if (= (player_pvar $cn logged_in) 1) [
        	setmaster $cn 0
		setpriv $cn none
        	player_pvar $cn logged_in 0
		player_pvar $cn adminlvl 0
		player_pvar $cn logged_in_as 0
        	msg (format "%1 %2" (blue (player_name $cn)) (orange [has been logged out]) )
        ] [privmsg $cn (format "%1" (red [You are not logged in!]) )]
]

playercmd_register = [
        local registername ""
        if (= (listlen $arguments) 3) [
        	statsdb eval [select name from register where name =$arg1] [registername = (column name)]
        	if (strcmp $registername $arg1) [
                	privmsg $cn (format "%1" (red [This username is already in use!]) )
        	] [
                	statsdb eval [INSERT INTO register VALUES(1, $arg1 , $arg2 , 0 , 0 , 0 , 'player' , 0);]
                	privmsg $cn (format "%1" (green [Your account is created, you can now login by typing #login name password]) )
                ]
        ] [privmsg $cn (format "%1" (red [You must enter: #register username password]) )]

]

playercmd_greet = [
        if (= (player_pvar $cn logged_in) 1) [
                logged_in_as_name = (player_pvar $cn logged_in_as)
                statsdb eval [update register set greet = $arg1 where name = $logged_in_as_name]
                privmsg $cn (format "%1 %2" (green [Your greet message changed to:Â]) $arg1 )
	] [privmsg $cn (format "%1" (red [You are not logged in!]) )]
]

playercmd_whoisonline = [
	if (= (player_pvar $cn logged_in) 1) [
		registcn = $cn
		privmsg $cn (format "%1     %2     %3" (green[NAME]) (blue[LOGINNAME]) (red[IP]) )
		foreach (players) [
			testcn = $arg1
			if (= (player_pvar $arg1 logged_in) 1) [
				msg (format "%1     %2     %3" (green (player_name $arg1)) (blue (player_pvar $arg1 logged_in_as)) (red (player_ip $arg1)) )
			]
		]  
	] [privmsg $cn (format "%1" (red [You are not logged in!]) )]
]

playercmd_changepw = [
	if (= (player_pvar $cn logged_in) 1) [
		 logged_in_as_name = (player_pvar $cn logged_in_as)
		 statsdb eval [update register set password = $arg1 where name = $logged_in_as_name ]
		 privmsg $cn (format "Your password successful changed to: %1" (red $arg1) )
	]
]

playercmd_changeuserpw = [
	local changeuserpwname ""
	if (&& (= (player_pvar $cn adminlvl) 1) (= (player_pvar $cn logged_in) 1)) [
	
	statsdb eval [select name from register where name =$arg1] [changeuserpwname = (column name)]
        if (strcmp $changeuserpwname $arg1) [
		statsdb eval [update register set password = $arg2 where name = $arg1]
		privmsg $cn (format "Command successful! Password from %1 changed to: %2 !" $arg1 $arg2)
	][privmsg $cn (format "%1 %2" (red[No user with name:]) (red $arg1) )]
	]
]


playercmd_giveadmin = [
	if (&& (= (player_pvar $cn adminlvl) 1) (= (player_pvar $cn logged_in) 1)) [
		statsdb eval [update register set admin ='1' where name = $arg1]
		privmsg $cn (format "Command successful! %1 has admin now!" $arg1)
	]
]

playercmd_deladmin = [
	local deladminname ""
	if (&& (= (player_pvar $cn adminlvl) 1) (= (player_pvar $cn logged_in) 1)) [	
	        statsdb eval [select name from register where name =$arg1] [deladminname = (column name)]
	        if (strcmp $deladminname $arg1) [
	                statsdb eval [update register set admin ='0' where name = $arg1]
			privmsg $cn (format "Command successful! %1 has no access privilege!" $arg1)
        	][privmsg $cn (format "%1 %2" (red[No user with name:]) (red $arg1) )]
	]	
]

playercmd_cmds = [ 
	privmsg $cn (format "%1" (blue[COMMAND		DESCRIPTION
#greet		Change your greet message when you log in
		    Example: #greet "Hello :)"
#changepw	Change your password
		    Example: #changepw newpassword
#whoisonline	Will show you the persons, who are logged in
#logout		Logout]) )
	if (= (player_pvar $cn adminlvl) 1) [
	privmsg $cn (format "%1" (blue[#getmaster	You will get master/admin
#leavemaster	You will leave master/admin
#changeuserpw	Change User's password
		    Example: #changeuserpw name password
#giveadmin	You will increase user level to admin permanently]) )
privmsg $cn (format "%1" (blue[#deladmin    The user level will be decreased]) )
	]
	privmsg $cn (format "%1" (red[

Open the console by pushing F11 to see the commandlist!!

]) )
]

playercmd_listuser = [
                 if (&& (= (player_pvar $cn adminlvl) 1) (= (player_pvar $cn logged_in) 1)) [
                        local registered_names ""
                        statsdb eval [select group_concat(name) from register] [registered_names = (column "group_concat(name)")]
                        privmsg $cn (format "%1 %2" (blue[All registered players on this Server, seperated by ',':]) (blue $registered_names) )
                 ] [privmsg $cn (red [Permission Denied]) ]
]

