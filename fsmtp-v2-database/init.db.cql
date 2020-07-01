-- Creates the keyspace
CREATE KEYSPACE IF NOT EXISTS fannst WITH REPLICATION = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
};