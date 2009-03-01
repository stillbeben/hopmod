
CREATE TABLE matches (
    id                  INTEGER PRIMARY KEY,
    datetime            TEXT,
    gamemode            TEXT,
    mapname             TEXT,
    duration            INTEGER,
    players             INTEGER
);

CREATE TABLE players (
    id                  INTEGER PRIMARY KEY,
    match_id            INTEGER REFERENCES matches(id),
    name                TEXT,
    team                TEXT,
    ipaddr              TEXT,
    frags               INTEGER DEFAULT 0,
    deaths              INTEGER DEFAULT 0,
    suicides            INTEGER DEFAULT 0,
    hits                INTEGER DEFAULT 0,
    misses              INTEGER DEFAULT 0,
    damage              INTEGER DEFAULT 0,
    kicked              BOOLEAN DEFAULT 0
);

CREATE TABLE itempickups (
    id                  INTEGER PRIMARY KEY,
    player_id           INTEGER REFERENCES players(id),
    boost               INTEGER DEFAULT 0,
    health              INTEGER DEFAULT 0,
    shells              INTEGER DEFAULT 0, 
    bullets             INTEGER DEFAULT 0,
    rockets             INTEGER DEFAULT 0,
    rounds              INTEGER DEFAULT 0,
    grenades            INTEGER DEFAULT 0,
    cartridges          INTEGER DEFAULT 0,
    greenarmor          INTEGER DEFAULT 0,
    yellowarmor         INTEGER DEFAULT 0,
    quad                INTEGER DEFAULT 0
);

CREATE TABLE gunusage (
    id                  INTEGER PRIMARY KEY,
    player_id           INTEGER REFERENCES players(id),
    pistol              INTEGER DEFAULT 0,
    rifle               INTEGER DEFAULT 0, 
    shotgun             INTEGER DEFAULT 0,
    chaingun            INTEGER DEFAULT 0,
    rocketlauncher      INTEGER DEFAULT 0,
    grenadelauncher     INTEGER DEFAULT 0
);

CREATE TABLE ctfteams (
    id                  INTEGER PRIMARY KEY,
    match_id            INTEGER REFERENCES matches(id),
    name                TEXT,
    score               INTEGER DEFAULT 0,
    pickups             INTEGER DEFAULT 0,
    drops               INTEGER DEFAULT 0,
    returns             INTEGER DEFAULT 0,
    resets              INTEGER DEFAULT 0
);

CREATE TABLE ctfplayers (
    id                  INTEGER PRIMARY KEY,
    player_id           INTEGER REFERENCES players(id),
    team_id             INTEGER REFERENCES ctfteams(id),
    pickups             INTEGER DEFAULT 0,
    drops               INTEGER DEFAULT 0,
    scored              INTEGER DEFAULT 0,
    defended            INTEGER DEFAULT 0,
    returns             INTEGER DEFAULT 0,
    teamkills           INTEGER DEFAULT 0
);

CREATE TABLE captureteams (
    id                  INTEGER PRIMARY KEY,
    match_id            INTEGER REFERENCES matches(id),
    name                TEXT,
    score               INTEGER DEFAULT 0,
    captured            INTEGER DEFAULT 0,
    lost                INTEGER DEFAULT 0,
    win                 BOOLEAN DEFAULT 0
);

CREATE TABLE captureplayers (
    id                  INTEGER PRIMARY KEY,
    player_id           INTEGER REFERENCES players(id),
    team_id             INTEGER REFERENCES captureteams(id),
    teamkills           INTEGER DEFAULT 0
);

CREATE TABLE playertotals (
    id                  INTEGER PRIMARY KEY,
    name                UNIQUE TEXT,
    ipaddr              TEXT,
    first_game_time     TEXT,
    last_game_time      TEXT,    
    frags               INTEGER DEFAULT 0,
    deaths              INTEGER DEFAULT 0,
    suicides            INTEGER DEFAULT 0,
    hits                INTEGER DEFAULT 0,
    misses              INTEGER DEFAULT 0,
    damage              INTEGER DEFAULT 0,
    kicked              BOOLEAN DEFAULT 0,
    ctf_pickups         INTEGER DEFAULT 0,
    ctf_drops           INTEGER DEFAULT 0,
    ctf_scored          INTEGER DEFAULT 0,
    ctf_defended        INTEGER DEFAULT 0,
    ctf_returns         INTEGER DEFAULT 0,
    teamkills           INTEGER DEFAULT 0,
    games               INTEGER DEFAULT 0
);

CREATE INDEX "player_id" on ctfplayers (player_id ASC);
CREATE INDEX "player_name" on players (name);
CREATE INDEX "player_ipaddr" on players (ipaddr);
CREATE INDEX "match_id" on players (match_id ASC);
CREATE INDEX "playertotals_by_name" playertotals (name ASC);

CREATE TRIGGER delete_player_cleanup AFTER DELETE ON players
BEGIN
    DELETE FROM itempickups WHERE player_id = old.id;
    DELETE FROM gunusage WHERE player_id = old.id;
    DELETE FROM ctfplayers WHERE player_id = old.id;
    DELETE FROM captureplayers WHERE player_id = old.id;
END;

CREATE TRIGGER delete_match_cleanup AFTER DELETE ON matches
BEGIN
    DELETE FROM ctfteams WHERE match_id = old.id;
    DELETE FROM captureteams WHERE match_id = old.id;
    DELETE FROM players where match_id = old.id;
END;

CREATE TRIGGER insert_playertotals AFTER INSERT ON players
BEGIN
    INSERT OR IGNORE INTO playertotals (name,first_game_time) VALUES (NEW.name,strftime('%s','now'));
END;
