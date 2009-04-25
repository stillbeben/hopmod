
CREATE TABLE domains (
    id          INTEGER PRIMARY KEY,
    name        TEXT UNIQUE,
    local       BOOLEAN DEFAULT 0
);

CREATE TABLE users (
    id          INTEGER PRIMARY KEY,
    domain_id   INTEGER REFERENCES domains(id),
    name        TEXT UNIQUE,
    pubkey      TEXT
);
