
CREATE TABLE IF NOT EXISTS servers (
    id          INTEGER PRIMARY KEY,
    servername  TEXT UNIQUE,
    hostname    TEXT,
    port        INTEGER DEFAULT 28787
);

CREATE TABLE IF NOT EXISTS domains (
    id          INTEGER PRIMARY KEY,
    server_id   INTEGER REFERENCES servers(id),
    name        TEXT UNIQUE,
    local       BOOLEAN DEFAULT 0
);

CREATE TABLE IF NOT EXISTS users (
    id          INTEGER PRIMARY KEY,
    domain_id   INTEGER REFERENCES domains(id),
    name        TEXT UNIQUE,
    pubkey      TEXT
);
