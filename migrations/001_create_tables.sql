PRAGMA foreign_keys=ON;

CREATE TABLE IF NOT EXISTS devices(
    address     INTEGER PRIMARY KEY,
    name        TEXT    NOT NULL,
    priority    INTEGER NOT NULL CHECK(priority IN (0,1,2)),
    is_faulty   INTEGER NOT NULL CHECK(is_faulty IN (0,1)),
    is_reserve  INTEGER NOT NULL CHECK(is_reserve IN (0,1)),
    updated_at  INTEGER NOT NULL DEFAULT (strftime('%s','now'))
);

CREATE TABLE IF NOT EXISTS workers(
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT    NOT NULL,
    max_jobs   INTEGER NOT NULL DEFAULT 1,
    skill      TEXT
);


CREATE TABLE IF NOT EXISTS jobs(
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    device_address  INTEGER NOT NULL REFERENCES devices(address) ON DELETE CASCADE,
    worker_id       INTEGER     REFERENCES workers(id) ON DELETE SET NULL,
    fault           TEXT    NOT NULL,
    status          INTEGER NOT NULL CHECK(status IN (0,1,2,3)),
    created_at      INTEGER NOT NULL DEFAULT (strftime('%s','now')),
    started_at      INTEGER,
    finished_at     INTEGER
);

CREATE INDEX IF NOT EXISTS idx_jobs_device  ON jobs(device_address);
CREATE INDEX IF NOT EXISTS idx_jobs_status  ON jobs(status);
