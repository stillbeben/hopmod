require "lunit"
require "mock_server"

local server = server
local dofile = dofile
local print = print
local os = require("os")
local math = require("math")

module( "capture_game_testcase", lunit.testcase )

server.stats_db_filename = ":memory:"
server.stats_debug = 1
statsmod = dofile("script/db/stats.lua")

function reset_db()
    statsmod.db:exec("DELETE FROM game")
    statsmod.db:exec("DELETE FROM teams")
    statsmod.db:exec("DELETE FROM players")
end

function test_normal()

    local p1 = server.newPlayer("graham","127.0.0.1")
    local p2 = server.newPlayer("unnamed","192.168.0.1")
    
    p1.iplong = 12701 -- not a proper value
    p2.iplong = 19201 -- ^
    
    server.initPlayerStats(p1)
    server.initPlayerStats(p2)
    
    server.gamemodeinfo.teams = true
    server.gamemodeinfo.capture = true
    server.timeleft = 15
    server.gamemode = "capture"
    server.map = "face-capture"
    
    p1.team = "good"
    p2.team = "evil"
    
    statsmod.setNewGame()
    
    p1.frags = 100
    p1.deaths = 50
    p1.hits = 100
    p1.shots = 200
    p1.damage = 100
    p1.win = true
    p1.timeplayed = 600
    p1.teamkills = 2
    
    p2.frags = 50
    p2.deaths = 100
    p2.hits = 50
    p2.shots = 100
    p2.damage = 50
    p2.win = false
    p2.timeplayed = 600
    server.timeleft = 0
    
    server.newTeam("good")
    server.team.good.score = 10
    server.team.good.win = true
    server.team.good.draw = false
    
    server.newTeam("evil")
    server.team.evil.score = 5
    server.team.evil.win = false
    server.team.evil.draw = false
    
    statsmod.commitStats()

    local row = statsmod.db:rows("SELECT * FROM players")
    
    local r1 = row()
    assert(r1)
    assert_equal(1, r1.id)
    assert_equal(1, r1.game_id)
    assert_equal(1, r1.team_id)
    assert_equal("graham",r1.name)
    assert_equal("127.0.0.1",r1.ipaddr)
    assert_equal(100, r1.frags)
    assert_equal(50, r1.deaths)
    assert_equal(0, r1.suicides)
    assert_equal(2, r1.teamkills)
    assert_equal(100, r1.hits)
    assert_equal(200, r1.shots)
    assert_equal(100, r1.damage)
    assert_equal(1, r1.win)
    assert_equal(1, r1.finished)
    assert_equal(600, r1.timeplayed)
    
    local r2 = row()
    assert(r2)
    assert_equal(2, r2.id)
    assert_equal(1, r2.game_id)
    assert_equal(2, r2.team_id)
    assert_equal("unnamed",r2.name)
    assert_equal("192.168.0.1",r2.ipaddr)
    assert_equal(50, r2.frags)
    assert_equal(100, r2.deaths)
    assert_equal(0, r2.suicides)
    assert_equal(0, r2.teamkills)
    assert_equal(50, r2.hits)
    assert_equal(100, r2.shots)
    assert_equal(50, r2.damage)
    assert_equal(0, r2.win)
    assert_equal(1, r2.finished)
    assert_equal(600, r2.timeplayed)
    
    assert_nil(row())
    
    row = statsmod.db:rows("SELECT * FROM games")
    
    r1 = row()
    assert(r1)
    assert_equal(1, r1.id)
    assert_equal("capture", r1.gamemode)
    assert_equal("face-capture", r1.mapname)
    assert_equal(15, r1.duration)
    assert_equal(2, r1.players)
    assert_true((math.abs(r1.datetime - os.time()) < 5))
    
    assert_nil(row())
    
    row = statsmod.db:rows("SELECT * FROM teams")
    r1 = row()
    assert(r1)
    assert_equal(1,r1.id)
    assert_equal(1,r1.game_id)
    assert_equal("good", r1.name)
    assert_equal(1, r1.win)
    assert_equal(0, r1.draw)
    
    r2 = row()
    assert(r2)
    assert_equal(2,r2.id)
    assert_equal(1,r2.game_id)
    assert_equal("evil", r2.name)
    assert_equal(0, r2.win)
    assert_equal(0, r2.draw)
    
    assert_nil(row())
    
    reset_db()
    
    server.removePlayer(1)
    server.removePlayer(2)
end
