import { Logger, LoggerLevel } from './logger';
import express from 'express';
import path from 'path';
import bodyParser from 'body-parser';
import accountsRoute from './routes/accounts.route';

const port: number = 80;

// Creates the express webserver, and adds the basic routes
const app: express.Application = express();
app.use(express.static(path.join(process.cwd(), "public")));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: false
}));

// Adds the routes
app.use('/accounts', accountsRoute);

// Starts listening
let logger: Logger = new Logger('Main', LoggerLevel.INFO);
app.listen(port, (err: Error) => {
  if (err) logger.error(err);
  else logger.print(`Server listening on port ${port}`);
});