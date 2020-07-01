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
    s_content TEXT,
    s_type INT,
    s_index INT,
    s_header frozen<list<fannst.email_header>>
);

-- Creates the email table
CREATE TABLE IF NOT EXISTS fannst.full_emails (
    e_user_uuid TIMEUUID,
    e_user_domain VARCHAR,
    e_uuid TIMEUUID,
    e_user_bucket BIGINT,
    e_email_rcv_date BIGINT,
    e_email_sent_date BIGINT,
    e_subject VARCHAR,
    e_type INT,
    e_transport_from fannst.email_address,
    e_transport_to fannst.email_address,
    e_from frozen<list<fannst.email_address>>,
    e_to frozen<list<fannst.email_address>>,
    e_headers frozen<list<fannst.email_header>>,
    e_body_sects frozen<list<fannst.email_body_section>>,
    PRIMARY KEY ((e_user_bucket), e_user_domain, e_user_uuid, e_uuid)
) WITH CLUSTERING ORDER BY (e_user_domain DESC, e_user_uuid DESC, e_uuid DESC);

CREATE TABLE IF NOT EXISTS fannst.local_domain (
    e_domain VARCHAR,
    e_domain_uuid TIMEUUID,
    PRIMARY KEY (e_domain, e_domain_uuid)
);