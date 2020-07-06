-- Creates the keyspace
CREATE KEYSPACE IF NOT EXISTS fannst WITH REPLICATION = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
};

INSERT INTO fannst.local_domain (e_domain, e_domain_uuid) VALUES ('fannst.nl', now())