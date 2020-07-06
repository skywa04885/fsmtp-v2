CREATE KEYSPACe measurements WITH REPLICATION = {'class': 'SimpleStrategy', 'replication_factor': 3};

USE measurements;

CREATE TYPE IF NOT EXISTS MEASUREMENT (
    m_voltage FLOAT,
    m_amperage FLOAT
);

CREATE TABLE IF NOT EXISTS measurements (
    m_date_year INT,
    m_date_day INT,
    m_date_month INT,
    m_time TIME,
    m_measurement MEASUREMENT,
    m_device VARCHAR,
    PRIMARY KEY((m_device), m_date_year, m_date_month, m_date_day, m_time)
) WITH CLUSTERING ORDER BY (m_date_year DESC, m_date_month DESC, m_date_day DESC, m_time DESC);

CREATE TABLE IF NOT EXISTS daily_average (
    d_date_year INT,
    d_date_month INT,
    d_date_day INT,
    d_device VARCHAR,
    d_average MEASUREMENT,
    PRIMARY KEY((d_device), d_date_year, d_date_month, d_date_day)
) WITH CLUSTERING ORDER BY (d_date_year DESC, d_date_month DESC, d_date_day DESC);

CREATE TABLE IF NOT EXISTS devices (
    d_device VARCHAR,
    d_remote_address VARCHAR,
    d_auth_key VARCHAR,
    d_device_uuid TIMEUUID,
    PRIMARY KEY(d_device_uuid)
);