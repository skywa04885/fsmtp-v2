import { Request, Response, NextFunction } from 'express';
import bcryptjs from 'bcryptjs';
import redisConnector from '../db/redis.db';
import redis from 'redis';
import { Logger, LoggerLevel } from '../logger';

const usernameRegex = /^[0-9A-Za-z._]{0,40}$/;
const emailRegex = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;

export function register(
  req: Request,
  res: Response,
  next: NextFunction
): void {
  const logger: Logger = new Logger(`Register:${req.connection.remoteAddress}`, LoggerLevel.INFO);

  // Validates the request data before proceeding to
  // - the user exists check
  if (
    !req.body.name ||
    !req.body.password ||
    !req.body.username ||
    !req.body.birthdate ||
    !req.body.domain
  ) {
    res.status(200).json({
      status: false,
      message: 'Some fields missing',
      fields: {
        name: !!req.body.name,
        password: !!req.body.password,
        username: !!req.body.username,
        birthdate: !!req.body.birthdate,
        domain: !!req.body.domain
      }
    });
    return;
  }
  
  if (!usernameRegex.test(req.body.username))
  {
    res.status(200).json({
      status: false,
      message: 'Invalid username'
    });
    return;
  }

  // Creates an connection to redis and checks if there is already
  // - an user under such name in the database
  logger.lprint('Connecting to redis, and checking if username and domain already in use.', LoggerLevel.DEBUG);
  const redisClient: redis.RedisClient = redisConnector();
  redisClient.hgetall(`acc:${req.body.username}@${req.body.domain}`, (err: any, map: any) => {
    // Checks if an error occured
    if (err)
    {
      logger.error(err);
      res.status(500).json({
        error: err.message
      });
      return;
    }
    
    // Checks if the value is not null, then we will
    // - send a response that the account already exists
    if (map)
    {
      res.status(200).json({
        status: false,
        message: `${req.body.username}@${req.body.domain} already exists`
      });
      return;
    }

    // Hashes the password using bcryptjs
    logger.lprint('User available, hashing password ...', LoggerLevel.DEBUG);
    bcryptjs.genSalt(10).then((salt: string) => {
      bcryptjs.hash(req.body.password, salt).then((hash: string) => {
        logger.lprint(`Password hashed, result: ${hash}`, LoggerLevel.DEBUG);
      }).catch((err: Error) => {
        logger.error(err);
        res.status(500).json({
          error: err.message
        });
      })
    }).catch((err: Error) => {
      logger.error(err);
      res.status(500).json({
        error: err.message
      });
    });
  });
}