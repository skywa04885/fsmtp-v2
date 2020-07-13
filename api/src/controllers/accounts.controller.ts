import express from 'express';
import cassandra from 'cassandra-driver';
import bcryptjs from 'bcryptjs';
import { AccountShortcut } from '../models/account-shortcut.model';
import { cassandraClientOptions } from '../options';
import { Account, AccountType } from '../models/account.model';
import { AES256GCM, EncryptionHandleType, EncryptionOperation } from '../security';
import randomstring from 'randomstring';

export function loginAccountPost(
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
): void
{
  let {
    a_domain,
    a_username,
    a_password
  } = req.body;
}

export function registerAccountPost(
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
): void
{
  let {
    a_full_name,
    a_username,
    a_domain,
    a_password
  } = req.body;

  // Checks the request body, and if
  // - the required params are there
  if (
    !a_full_name ||
    !a_username ||
    !a_domain ||
    !a_password
  )
  {
    res.status(200).json({
      status: false,
      message: 'missing fields',
      fields: {
        a_full_name: !!a_full_name,
        a_username: !!a_username,
        a_domain: !!a_domain,
        a_password: !!a_password
      }
    });
    return;
  }

  // Creates an cassandra connection
  let cass: cassandra.Client = new cassandra.Client(cassandraClientOptions);

  // Checks if there is already an
  // - existing user under that domain
  AccountShortcut.find(cass, a_domain, a_username)
    .then((acc: AccountShortcut) => {
      if (acc !== undefined)
      {
        res.status(200).json({
          status: false,
          message: 'email already in use'
        });
        return;
      }

      // Creates the encryption key from the password
      // - and generates the accout key
      AES256GCM.generateKey(a_password)
        .then(({key, salt}) => {
          const rawKey: string = randomstring.generate(128);
          AES256GCM.handle(
            rawKey,
            key,
            EncryptionHandleType.ENCRYPT,
            EncryptionOperation.ENCRYPT_TO_BASE64
          ).then(({data, gas}) => {

            // Performs the hashing of the password
            // - so data leaks will not suck
            bcryptjs.genSalt(10)
              .then((pSalt: string) => {
                bcryptjs.hash(a_password, pSalt)
                  .then((pHash: string) => {
                    // Creates the account, and
                    // - stores the account
                    let account: Account = new Account(
                      a_username,
                      'def',
                      pHash,
                      a_domain,
                      Account.getBucket(),
                      a_full_name,
                      new Date(),
                      new Date(),
                      data,
                      salt,
                      5.0,
                      '', '', '', '', '',
                      AccountType.AT_FREE,
                      cassandra.types.TimeUuid.now(),
                      0x0,
                      0,
                      1 * 1000 * 1000 * 1000
                    );
                    account.save(cass)
                      .then(() => {
                        res.status(500).json({
                          status: true
                        });
                      })
                      .catch((err: Error) => {
                        res.status(500).json({
                          status: false,
                          message: err.toString()
                        });
                      });
                  }).catch((err: Error) => {
                    res.status(500).json({
                      status: false,
                      message: err.toString()
                    });
                  });
              }).catch((err: Error) => {
                res.status(500).json({
                  status: false,
                  message: err.toString()
                });
              });
          });
        }).catch((err: Error) => {
          res.status(500).json({
            status: false,
            message: err.toString()
          });
        });
    })
    .catch((err: Error) => {
      res.status(500).json({
        status: false,
        message: err.toString()
      });
    });
};