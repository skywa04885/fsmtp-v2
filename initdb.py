#/bin/python

import json
import os
from cassandra.cluster import Cluster
from cassandra.auth import PlainTextAuthProvider

queries = [
  # Creates the keyspace
  '''CREATE KEYSPACE IF NOT EXISTS fannst WITH REPLICATION = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
  };''',
  # Creates the tables for the raw emails
  '''CREATE TABLE IF NOT EXISTS fannst.raw_emails (
    e_bucket BIGINT,
    e_domain VARCHAR,
    e_owners_uuid TIMEUUID,
    e_email_uuid TIMEUUID,
    e_content TEXT,
    PRIMARY KEY ((e_bucket), e_domain, e_owners_uuid, e_email_uuid)
  ) WITH CLUSTERING ORDER BY (e_domain DESC, e_owners_uuid DESC, e_email_uuid DESC);''',
  # Creates the table for the email shortcuts
  '''CREATE TABLE IF NOT EXISTS fannst.email_shortcuts (
    e_domain VARCHAR,
    e_subject VARCHAR,
    e_preview VARCHAR,
    e_owners_uuid TIMEUUID,
    e_email_uuid TIMEUUID,
    e_bucket BIGINT,
    e_size_octets BIGINT,
    e_uid INT,
    e_from VARCHAR,
    e_flags INT,
    e_mailbox VARCHAR,
    PRIMARY KEY((e_domain), e_owners_uuid, e_mailbox, e_email_uuid)
  ) WITH CLUSTERING ORDER BY(e_owners_uuid DESC, e_mailbox DESC, e_email_uuid DESC);''',
  # Creates the table for the domain registration
  '''CREATE TABLE IF NOT EXISTS fannst.local_domain (
    e_domain VARCHAR,
    e_domain_uuid TIMEUUID,
    PRIMARY KEY (e_domain, e_domain_uuid)
  );''',
  # Creates the table for the mailboxes
  '''CREATE TABLE IF NOT EXISTS fannst.mailboxes (
    e_bucket BIGINT,
    e_domain VARCHAR,
    e_uuid TIMEUUID,
    e_mailbox_path VARCHAR,
    e_mailbox_stand BOOLEAN,
    e_message_count INT,
    e_flags INT,
    e_subscribed BOOLEAN,
    PRIMARY KEY((e_bucket), e_domain, e_uuid, e_mailbox_path)
  ) WITH CLUSTERING ORDER BY (e_domain DESC, e_uuid DESC, e_mailbox_path DESC);''',
  # Creates the accounts tabke
  '''CREATE TABLE IF NOT EXISTS fannst.accounts (
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
  ''',
  # Creates the account shortcut table
  '''CREATE TABLE IF NOT EXISTS fannst.account_shortcuts (
      a_bucket BIGINT,
      a_domain VARCHAR,
      a_uuid TIMEUUID,
      a_username VARCHAR,
      PRIMARY KEY ((a_domain), a_username)
  ) WITH CLUSTERING ORDER BY (a_username DESC);''',
  # Adds fannst to the local domains
  '''INSERT INTO fannst.local_domain (e_domain, e_domain_uuid) VALUES ('fannst.nl', now());'''
]

'''
Reads the json configuration, after which we will connect
 to cassandra and initialize the database
'''

config = ''
if os.path.exists('../config.json'):
  config = '../config.json'
else:
  config = '../fallback/config.json'

cass_username = None
cass_password = None
cass_hosts = None
cass_native = None
with open(config, 'r') as f:
  data = json.load(f)

  cass_username = data['database']['cassandra_username']
  cass_password = data['database']['cassandra_password']
  cass_hosts = data['database']['cassandra_hosts']
  cass_native = data['database']['cassandra_native']

'''
Connects to cassandra and creates the database and tables
'''

auth = None
if cass_username != None and cass_password != None:
  auth = PlainTextAuthProvider(username=cass_username, password=cass_password)

cluster = Cluster(cass_hosts.split(','), port=cass_native, auth_provider=auth)
session = cluster.connect()

for query in queries:
  print('Executing:', query)
  session.execute(query)

