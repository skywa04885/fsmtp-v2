import redis from 'redis';
import { redisHost } from '../config';
import { Logger, LoggerLevel } from '../logger';

export default function connect() {
  const logger: Logger = new Logger('Redis', LoggerLevel.INFO);
  const client: redis.RedisClient = redis.createClient({
    host: redisHost
  });

  client.on('error', (err: Error) => {
    logger.error(err);
  });

  return client;
}