-- Creates the users table
CREATE TABLE IF NOT EXISTS fannst.users (
    u_uuid TIMEUUID,
    u_password VARCHAR,
    u_full_name VARCHAR,
);

CREATE TYPE IF NOT EXISTS fannst.user_reference (
    u_uuid TIMEUUID,
    u_bucket TIMEUUID,
    u_domain VARCHAR
);