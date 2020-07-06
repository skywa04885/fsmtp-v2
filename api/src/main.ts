const port: number = 8080;

import express from 'express';

let app: express.Application = express();

app.listen(8080, () => {
	console.log(`FSMTP Api ready on port ${port}`)
});