import crypto from 'crypto';
import { promises } from 'fs';

const algorithm = 'aes-256-gcm';

export const gasPrices: {
  encryption: number,
  decryption: number
} = {
  encryption: 0.0002,
  decryption: 0.0001
};

// ==== The handle type ====
export enum EncryptionHandleType {
  ENCRYPT = 0,
  DECRYPT
};

// ==== The operation type ====
export enum EncryptionOperation {
  ENCRYPT_TO_BASE64 = 0,
  DECRYPT_FROM_BASE64_TO_PLAIN_TEXT,
  DECRYPT_FROM_BASE64_TO_JSON,
  DECRYPT_OBJECT_FROM_BASE64_TO_PLAIN
};

export class AES256GCM
{
  /**
   * Generates the key from the passphrase, and existing salt -
   * If no salt supplied create a new salt
   * @Param passphrase
   * @Param salt
   */
  public static generateKey = (
    passphrase: string, 
    salt: string = crypto.randomBytes(24).toString('base64').substring(0, 24)
  ): Promise<{
    key: Buffer,
    salt: string
  }> => {
    return new Promise((resolve, reject) => {
      // ==== Checks if there is an passphrase ====
      if (!passphrase) return reject(new Error('Passphrase may not be empty !'));

      // ==== Creates the Key ====
      crypto.scrypt(passphrase, salt, 32, (err, key) => {
        // Checks for error
        if (err) return reject(err);

        // Returns the key and salt ( If generated ofcourse )
        resolve({
          key: key,
          salt: salt
        });
      });
    });
  }

  public static decrypt = (data: string, start: number, key: Buffer): Promise<{
    data: any,
    gas: number
  }> =>
  {
    return new Promise((resolve, reject) => {
      // ==== Parses the encrypted text ====

      let dotPosition: number = data.indexOf('.');
      let lastDotPosition: number = data.lastIndexOf('.');

      // Gets the encrypted data
      const encrypted: Buffer = Buffer.from(data.substring(0, dotPosition), 'base64');
      const iv: Buffer = Buffer.from(data.substring(dotPosition + 1, lastDotPosition), 'base64');
      const authTag: Buffer = Buffer.from(data.substring(lastDotPosition + 1), 'base64');

      // ==== Starts decrypting the data ====

      // The decrypted result
      let decrypted: string = '';
      // Gets the cipher
      const decipher: any = crypto.createDecipheriv(algorithm, key, iv);
      // Sets the auth  tag
      decipher.setAuthTag(authTag);

      // Sets the readable listener
      decipher.on('readable', () => {
        let chunk;
        while (null !== (chunk = decipher.read()))
          decrypted += chunk.toString('utf-8');
      });

      // Sets the end listener
      decipher.on('end', () => {
        resolve({
          gas: (Date.now() - start) * gasPrices.decryption,
          data: decrypted
        })
      });

      // Writes the encrypted data
      decipher.write(encrypted);
      decipher.end();
    });
  }

  public static handle = (
    data: any,
    key: Buffer,
    type: EncryptionHandleType,
    retType: EncryptionOperation = EncryptionOperation.DECRYPT_FROM_BASE64_TO_PLAIN_TEXT
  ): Promise<{
    data: any,
    gas: number
  }> => {
    return new Promise((resolve, reject) => {
      // Sets the start time
      const start: number = Date.now();

      // ==== Checks if it should encrypt or decrypt ====
      switch (type)
      {
        // ==== Handles encryption ====
        case EncryptionHandleType.ENCRYPT:
        {
          // ==== Creates the cipher ====

          // Creates the IV
          const iv: Buffer = new Buffer(crypto.randomBytes(16).toString('utf8'), 'utf8');
          // Creates the cipher
          const cipher: any = crypto.createCipheriv(algorithm, key, iv);

          // ==== Sets the tostirng type ====
          let toStringType: string;
          switch (retType)
          {
            case EncryptionOperation.ENCRYPT_TO_BASE64:
            {
              toStringType = 'base64';
              break;
            }
            default: return reject(new Error('Encryption operation not supported !'));
          }

          // ==== Encrypts the data ====

          // Creates the encrypted result
          let encrypted: string = '';

          // Adds the readable listener
          cipher.on('readable', () => {
            let chunk: any = undefined;
            while (null !== (chunk = cipher.read()))
              encrypted += chunk.toString(toStringType);
          });

          // Adds the end listener
          cipher.on('end', () => {
            // Returns the data
            resolve({
              gas: (Date.now() - start) * gasPrices.encryption,
              data: encrypted + '.' + iv.toString('base64') + '.' + cipher.getAuthTag().toString('base64')
            });
          });

          // Writes the data to the cipher
          cipher.write(Buffer.from(data, 'utf-8'));
          cipher.end();
          
          // Breaks
          break;
        }
        // ==== Handles decryption ====
        case EncryptionHandleType.DECRYPT:
        {
          switch (retType)
          {
            // ==== Handles plain text decryption ====
            case EncryptionOperation.DECRYPT_FROM_BASE64_TO_PLAIN_TEXT:
            {
              // Decrypts the data
              AES256GCM.decrypt(data, start, key)
                .then(({data, gas}) => resolve({data, gas}))
                .catch((err: Error) => reject(err));
              
              // Breaks
              break;
            }
            // ==== Handles object decryption ====
            case EncryptionOperation.DECRYPT_OBJECT_FROM_BASE64_TO_PLAIN:
            {
              // Creates the operations array
              let operations: Promise<{data: any, gas: number}>[] = [];

              // Creates the operations
              for (let feature in data)
                operations.push(AES256GCM.decrypt(
                  data[feature],
                  start,
                  key
                ));

              // Executes the promises
              Promise.all(operations).then(results => {
                // Stores the loop index
                let index: number = 0;
                let keys: string[] = Object.keys(data);
                let returnData: {
                  data: any,
                  gas: number
                } = {
                  data: {},
                  gas: 0.0
                };

                // Calculates the gas usage
                results.forEach((result: {data: any, gas: number}) => {
                  // Adds the gas
                  returnData.gas += result.gas;
                  returnData.data[keys[index]] = result.data;

                  // Increments the index
                  index++;
                });
                
                // Returns
                resolve(returnData);
              });

              // Calls the first round
              break;
            }
          }
          break;
        }
      }
    });
  };
};