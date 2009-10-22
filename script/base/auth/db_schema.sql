
CREATE TABLE IF NOT EXISTS domains (
    id          INTEGER PRIMARY KEY,
    name        TEXT UNIQUE
);

CREATE TABLE IF NOT EXISTS users (
    id          INTEGER PRIMARY KEY,
    domain_id   INTEGER REFERENCES domains(id),
    name        TEXT UNIQUE,
    pubkey      TEXT
);
