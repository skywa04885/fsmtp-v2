import cassandra, { types } from 'cassandra-driver';
import { AccountShortcut } from './account-shortcut.model';

export const enum AccountType {
	AT_FREE,
	AT_PREMIUM,
	AT_BUSINESS,
	AT_LIMITLESS
}; 

export class Account
{
	a_Username: string;
	a_PictureURI: string;
	a_Password: string;
	a_Domain: string;
	a_Bucket: number;
	a_FullName: string;
	a_BirthDate: Date;
	a_CreationDate: Date;
	a_AccountKey: string;
	a_AccountKeySalt: string;
	a_Gas: number;
	a_Country: string;
	a_Region: string;
	a_City: string;
	a_Address: string;
	a_Phone: string;
	a_Type: AccountType;
	a_UUID: types.TimeUuid;
	a_Flags: number;
	a_StorageUsageBytes: number;
	a_MaxStorageBytes: number;

	/**
	 * Default constructor for the Account
	 * 
	 * @param {string} a_Username
	 * @param {string} a_PictureURI;
	 * @param {string} a_Password
	 * @param {string} a_Domain
	 * @param {number} a_Bucket
	 * @param {string} a_FullName
	 * @param {Date} a_BirthDate
	 * @param {Date} a_CreationDate
	 * @param {string} a_AccountKey
	 * @param {string} a_AccountKeySalt
	 * @param {number} a_Gas
	 * @param {string} a_Country
	 * @param {string} a_Region
	 * @param {string} a_City
	 * @param {string} a_Address
	 * @param {string} a_Phone
	 * @param {AccountType} a_Type
	 * @param {types.TimeUuid} a_UUID
	 * @param {number} a_Flags
	 * @param {number} a_StorageUsageBytes
	 * @param {number} a_MaxStorageBytes
	 */
	public constructor(
		a_Username: string,
		a_PictureURI: string,
		a_Password: string,
		a_Domain: string,
		a_Bucket: number,
		a_FullName: string,
		a_BirthDate: Date,
		a_CreationDate: Date,
		a_AccountKey: string,
		a_AccountKeySalt: string,
		a_Gas: number,
		a_Country: string,
		a_Region: string,
		a_City: string,
		a_Address: string,
		a_Phone: string,
		a_Type: AccountType,
		a_UUID: types.TimeUuid,
		a_Flags: number,
		a_StorageUsageBytes: number,
		a_MaxStorageBytes: number
	)
	{
		this.a_Username = a_Username;
		this.a_PictureURI = a_PictureURI;
		this.a_Password = a_Password;
		this.a_Domain = a_Domain;
		this.a_Bucket = a_Bucket;
		this.a_FullName = a_FullName;
		this.a_BirthDate = a_BirthDate;
		this.a_CreationDate = a_CreationDate;
		this.a_AccountKey = a_AccountKey;
		this.a_AccountKeySalt = a_AccountKeySalt;
		this.a_Gas = a_Gas;
		this.a_Country = a_Country;
		this.a_Region = a_Region;
		this.a_City = a_City;
		this.a_Address = a_Address;
		this.a_Phone = a_Phone;
		this.a_Type = a_Type;
		this.a_UUID = a_UUID;
		this.a_Flags = a_Flags;
		this.a_StorageUsageBytes = a_StorageUsageBytes;
		this.a_MaxStorageBytes = a_MaxStorageBytes;
	}

	static fromMap(map: any): Account
	{
		return new Account(
			map['a_username'],
			map['a_picture_uri'],
			map['a_password'],
			map['a_domain'],
			map['a_bucket'],
			map['a_full_name'],
			map['a_birth_date'],
			map['a_creation_date'],
			map['a_account_key'],
			map['a_account_key_salt'],
			map['a_gas'],
			map['a_country'],
			map['a_region'],
			map['a_city'],
			map['a_address'],
			map['a_phone'],
			map['a_type'],
			map['a_uuid'],
			map['a_flags'],
			map['a_storage_used_bytes'],
			map['a_storage_max_bytes']
		);
	}

	save(cass: cassandra.Client): Promise<void>
	{
		return new Promise((resolve, reject) => {
			const query: string = /*sql*/`INSERT INTO fannst.accounts (
				a_username, a_picture_uri, a_password,
				a_domain, a_bucket, a_full_name,
				a_birth_date, a_creation_date, a_account_key,
				a_gas, a_country, a_region,
				a_city, a_address, a_phone,
				a_type, a_uuid, a_flags,
				a_storage_used_kb, a_storage_max_bytes, e_account_key_salt
			) VALUES (
				?, ?, ?,
				?, ?, ?,
				?, ?, ?,
				?, ?, ?,
				?, ?, ?,
				?, ?, ?,
				?, ?, ?
			)`;

			cass.execute(query, [
				this.a_Username,
				this.a_PictureURI,
				this.a_Password,
				this.a_Domain,
				this.a_Bucket,
				this.a_FullName,
				this.a_BirthDate,
				this.a_CreationDate,
				this.a_AccountKey,
				this.a_Gas,
				this.a_Country,
				this.a_Region,
				this.a_City,
				this.a_Address,
				this.a_Phone,
				this.a_Type,
				this.a_UUID,
				this.a_Flags,
				this.a_StorageUsageBytes,
				this.a_MaxStorageBytes,
				this.a_AccountKeySalt
			], {
				prepare: true
			}).then(() => {
				// Creates the account shortcut, and
				// - saves it to the database
				let shortcut: AccountShortcut = new AccountShortcut(
					this.a_Bucket,
					this.a_Domain,
					this.a_UUID,
					this.a_Username
				);
				shortcut.save(cass)
					.then(() => resolve())
					.catch((err: Error) => reject(err));
			})
			.catch((err: Error) => reject(err));
		});
	}

	static get(
		cass: cassandra.Client,
		a_bucket: number,
		a_domain: string,
		a_uuid: types.TimeUuid
	): Promise<Account>
	{
		return new Promise((resolve, reject) => {
			const query: string = /*sql*/`SELECT * FROM fannst.accounts WHERE a_bucket=? AND a_domain=? AND a_uuid=?`;

			cass.execute(query, [
				a_bucket,
				a_domain,
				a_uuid
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

	static getBucket(): number
	{
		return Math.round(Date.now() / 10 / 1000);
	}
}