#!/bin/python

'''
FSMTP-V2 SSLGen

Generates an self-signed SSL Certificate and key
'''

from OpenSSL import crypto, SSL

config = {
    'email_address': 'someone@example.com',
    'common_name': 'example.com',
    'country_name': 'CN',
    'locality_name': 'Locality',
    'state_name': 'State',
    'organization_name': 'Organization',
    'organization_unit': 'Unit',
    'serial_number': 0,
    'validity_start_in_seconds': 0,
    'validity_end_in_seconds': 10 * 365 * 24 * 60 * 60,
    'cert_out': './env/keys/cert.pem',
    'key_out': './env/keys/key.pem'
}

# Generates the key
key = crypto.PKey();
key.generate_key(crypto.TYPE_RSA, 4096)

# Generates the certificate
cert = crypto.X509();
cert.get_subject().C = config['country_name']
cert.get_subject().ST = config['state_name']
cert.get_subject().L = config['locality_name']
cert.get_subject().O = config['organization_name']
cert.get_subject().OU = config['organization_unit']
cert.get_subject().CN = config['common_name']
cert.get_subject().emailAddress = config['email_address']
cert.set_serial_number(config['serial_number'])
cert.gmtime_adj_notBefore(config['validity_start_in_seconds'])
cert.gmtime_adj_notAfter(config['validity_end_in_seconds'])
cert.set_issuer(cert.get_subject())
cert.set_pubkey(key)
cert.sign(key, 'sha512')

with open(config['cert_out'], 'w+') as f:
    f.write(crypto.dump_certificate(crypto.FILETYPE_PEM, cert).decode('utf-8'))
with open(config['key_out'], 'w+') as f:
    f.write(crypto.dump_privatekey(crypto.FILETYPE_PEM, key).decode('utf-8'))

