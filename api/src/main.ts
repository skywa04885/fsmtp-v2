const port: number = 8080;

import express from 'express';
import path from 'path';
import bodyParser from 'body-parser';

import accountsRoute from './routes/accounts.route';

let app: express.Application = express();

app.set('view engine', 'ejs');
app.set('views', path.join(process.cwd(), 'views'));
app.use(express.static(path.join(process.cwd(), 'public', 'dist')));
app.use(express.static(path.join(process.cwd(), 'public', 'static')));
app.use(bodyParser.json())

app.use('/accounts', accountsRoute);

app.get('/', (req, res, next) => res.redirect('/accounts/register'));

app.listen(8080, () => {
	console.log(`FSMTP Api ready on port ${port}`)
});