-- Creates the email header type
CREATE TYPE IF NOT EXISTS fannst.email_header (
    h_key VARCHAR,
    h_value VARCHAR
);

-- Creates the email address type
CREATE TYPE IF NOT EXISTS fannst.email_address (
    a_name VARCHAR,
    a_address VARCHAR
);

-- Creates the email body section type
CREATE TYPE IF NOT EXISTS fannst.email_body_section (
    e_content TEXT,
    e_type INT,
    e_index INT,
    e_header FROZEN<LIST<fannst.email_header>>,
    e_transfer_encoding INT
);

-- Creates the email table
CREATE TABLE IF NOT EXISTS fannst.full_emails (
    e_transport_from email_address,
    e_transport_to email_Address,
    e_from FROZEN<LIST<email_address>>,
    e_to FROZEN<LIST<email_address>>,
    e_domain VARCHAR,
    e_subject VARCHAR,
    e_message_id VARCHAR,
    e_body_sections FROZEN<LIST<email_body_section>>,
    e_headers FROZEN<LIST<email_header>>,
    e_encrypted BOOLEAN,
    e_date BIGINT,
    e_owners_uuid TIMEUUID,
    e_email_uuid TIMEUUID,
    e_type INT,
    e_bucket BIGINT,
    PRIMARY KEY ((e_bucket), e_domain, e_type, e_owners_uuid, e_email_uuid)
) WITH CLUSTERING ORDER BY (e_domain DESC, e_type DESC, e_owners_uuid DESC, e_email_uuid DESC);

CREATE TABLE IF NOT EXISTS fannst.raw_emails (
    e_bucket BIGINT,
    e_domain VARCHAR,
    e_owners_uuid TIMEUUID,
    e_email_uuid TIMEUUID,
    e_content TEXT,
    PRIMARY KEY ((e_bucket), e_domain, e_owners_uuid, e_email_uuid)
) WITH CLUSTERING ORDER BY (e_domain DESC, e_owners_uuid DESC, e_email_uuid DESC);

-- Creates the email index table (per user email index)
CREATE TABLE IF NOT EXISTS fannst.email_shortcuts (
    e_domain VARCHAR,
    e_subject VARCHAR,
    e_preview VARCHAR,
    e_owners_uuid TIMEUUID,
    e_email_uuid TIMEUUID,
    e_bucket BIGINT,
    e_size_octets BIGINT,
    e_uid INT,
    e_mailbox VARCHAR,
    PRIMARY KEY((e_domain), e_owners_uuid, e_mailbox, e_email_uuid)
) WITH CLUSTERING ORDER BY(e_owners_uuid DESC, e_mailbox DESC, e_email_uuid DESC);

CREATE TABLE IF NOT EXISTS fannst.local_domain (
    e_domain VARCHAR,
    e_domain_uuid TIMEUUID,
    PRIMARY KEY (e_domain, e_domain_uuid)
);

-- The MAIL folders
CREATE TABLE IF NOT EXISTS fannst.mailboxes (
    e_bucket BIGINT,
    e_domain VARCHAR,
    e_uuid TIMEUUID,
    e_mailbox_path VARCHAR,
    e_mailbox_stand BOOLEAN,
    e_message_count INT,
    e_flags INT,
    e_subscribed BOOLEAN,
    PRIMARY KEY((e_bucket), e_domain, e_uuid, e_mailbox_path)
) WITH CLUSTERING ORDER BY (e_domain DESC, e_uuid DESC, e_mailbox_path DESC);
