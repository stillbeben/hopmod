local domain = server.name_reservation_domain
local reserved_name_expire = server.reserved_name_expire

local failure_message = {}
failure_message[auth.request_status.REQUEST_FAILED] = "You are using a reserved name."
failure_message[auth.request_status.CHALLENGE_FAILED] = "The server failed to authenticate you for the use of the reserved name."
failure_message[auth.request_status.RESPONSE_FAILED] = failure_message[auth.request_status.CHALLENGE_FAILED]
failure_message[auth.request_status.TIMEOUT] = failure_message[auth.request_status.CHALLENGE_FAILED]
failure_message["WRONG_KEY"] = "You authenticated with a key for another user."

local function player_verified(cn)

	local pvars = server.player_vars(cn)

	return pvars.name_verified and pvars.reserved_name and pvars.nameprotect_wanted_authname and pvars.reserved_name_expire and string.lower(pvars.reserved_name) == string.lower(pvars.nameprotect_wanted_authname) and tonumber(server.uptime) < pvars.reserved_name_expire

end


local function check_name(cn)

	local name = server.player_name(cn)

	if not player_verified(cn) then 
		server.player_vars(cn).nameprotect_wanted_authname = name

		auth.query_id(string.lower(name), domain, function(result)

			if not result or result == false then
				return
			end

			if result == true then
				auth.send_request(cn, domain, function(cn, user_id, domain, status)

					local pvars = server.player_vars(cn)

					if status == auth.request_status.SUCCESS then
						if string.lower(pvars.nameprotect_wanted_authname) == string.lower(user_id) then
							server.log(string.format("%s(%i) authenticated as '%s' to use reserved name.", server.player_name(cn), cn, user_id))

							local vars = server.player_vars(cn)

							pvars.name_verified = true
							pvars.reserved_name = user_id
							pvars.reserved_name_expire = tonumber(server.uptime) + tonumber(reserved_name_expire)

							vars.name_verified = true
							vars.reserved_name = user_id
							vars.reserved_name_expire = tonumber(server.uptime) + tonumber(reserved_name_expire)

							return
						else
							status = "WRONG_KEY"
						end
					end

					if status == auth.request_status.CANCELLED then
						return
					end

					server.player_msg(cn, red(failure_message[status]))
					server.player_msg(cn, "You have 10 seconds to rename.")

					local session_id = server.player_sessionid(cn)
					local old_name = server.player_name(cn)

					server.sleep(10000, function()

						if server.player_sessionid(cn) ~= session_id then
							return
						end

						if server.player_name(cn) == old_name then
							server.kick(cn, 0, "server", "use of reserved name")
						end

					end)

				end)

			end

		end)

	end

end

server.event_handler("connect", check_name)

