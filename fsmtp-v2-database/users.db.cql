-- Creates the users table
CREATE TABLE IF NOT EXISTS fannst.accounts (
    a_username VARCHAR,
    a_picture_uri VARCHAR,
    a_password VARCHAR,
    a_domain VARCHAR,
    a_bucket BIGINT,
    a_full_name VARCHAR,
    a_birth_date BIGINT,
    a_creation_date BIGINT,
    a_rsa_public TEXT,
    a_rsa_private TEXT,
    a_gas DOUBLE,
    a_country VARCHAR,
    a_region VARCHAR,
    a_city VARCHAR,
    a_address VARCHAR,
    a_phone VARCHAR,
    a_type TINYINT,
    a_uuid TIMEUUID,
    a_flags BIGINT,
    a_storage_used_bytes BIGINT,
    a_storage_max_bytes BIGINT,
    PRIMARY KEY ((a_bucket), a_domain, a_uuid)
) WITH CLUSTERING ORDER BY (a_domain DESC, a_uuid DESC);

CREATE TABLE IF NOT EXISTS fannst.account_shortcuts (
    a_bucket BIGINT,
    a_domain VARCHAR,
    a_uuid TIMEUUID,
    a_username VARCHAR,
    PRIMARY KEY ((a_domain), a_username)
) WITH CLUSTERING ORDER BY (a_username DESC);