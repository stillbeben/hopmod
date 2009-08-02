
CREATE TABLE domains (
    id          INTEGER PRIMARY KEY,
    name        TEXT UNIQUE,
    local       BOOLEAN DEFAULT 0
);

CREATE TABLE tags (
    id          INTEGER PRIMARY KEY,
    name     	TEXT UNIQUE,
    place	INTEGER,
    tag_front	TEXT,
    tag_back	TEXT
);

CREATE TABLE users (
    id                INTEGER PRIMARY KEY,
    domain_id         INTEGER REFERENCES domains(id),
    name              TEXT,
    pubkey            TEXT,
    clan_id           TEXT REFERENCES tags(id)
);

CREATE INDEX "player_name" ON users (name ASC);
CREATE INDEX "tag_name" ON tags (name ASC);
CREATE INDEX "tag_front_name" ON tags (tag_front ASC);
CREATE INDEX "tag_back_name" ON tags (tag_back ASC);
