import cassandra, { types } from 'cassandra-driver';

export class AccountShortcut
{
  a_Bucket: number;
  a_Domain: string;
  a_UUID: types.TimeUuid;
  a_Username: string;

  public constructor(
    a_Bucket: number,
    a_Domain: string,
    a_UUID: types.TimeUuid,
    a_Username: string
  )
  {
    this.a_Bucket = a_Bucket;
    this.a_Domain = a_Domain;
    this.a_UUID = a_UUID;
    this.a_Username = a_Username;
  }

  public static fromMap(map: any): AccountShortcut
  {
    return new AccountShortcut(
      map['a_bucket'],
      map['a_domain'],
      map['a_uuid'],
      map['a_username']
    );
  }

  public static find(cass: cassandra.Client, a_domain: string, a_username: string): Promise<AccountShortcut>
  {
    return new Promise((resolve, reject) => {
      const query: string = /*sql*/`SELECT a_uuid, a_bucket FROM fannst.account_shortcuts WHERE a_domain=? AND a_username=?`;

      cass.execute(query, [
        a_domain,
        a_username
      ], {
        prepare: true
      }).then((res: any) => {
        // Checks if the result contains an user, if so
				// - return it, else undefined
				if (!res.rows[0]) resolve(undefined);
				else return this.fromMap(res.rows[0]);
      }).catch((err: Error) => reject(err));
    });
  }

  public save(cass: cassandra.Client): Promise<void>
  {
    return new Promise((resolve, reject) => {
      const query: string = /*sql*/`INSERT INTO fannst.account_shortcuts (
        a_bucket, a_domain, a_uuid,
        a_username
      ) VALUES (
        ?, ?, ?,
        ?
      )`;

      cass.execute(query, [
        this.a_Bucket,
        this.a_Domain,
        this.a_UUID,
        this.a_Username
      ], {
        prepare: true
      }).then(() => resolve())
      .catch((err: Error) => reject(err));
    });
  }
}