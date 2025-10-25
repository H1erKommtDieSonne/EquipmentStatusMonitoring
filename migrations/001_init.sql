CREATE TABLE IF NOT EXISTS schema_version (version INTEGER NOT NULL);
INSERT INTO schema_version(version)
SELECT 1 WHERE NOT EXISTS(SELECT 1 FROM schema_version);

CREATE TABLE IF NOT EXISTS workers (
  worker_id   INTEGER PRIMARY KEY AUTOINCREMENT,
  name        TEXT NOT NULL,
  skill       TEXT,
  is_active   INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS devices (
  address     INTEGER PRIMARY KEY,
  name        TEXT NOT NULL,
  priority    INTEGER NOT NULL,
  is_faulty   INTEGER NOT NULL,
  is_reserve  INTEGER NOT NULL
);
