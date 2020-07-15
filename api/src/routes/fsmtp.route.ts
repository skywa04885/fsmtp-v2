import express from 'express';
import path from 'path';
import { prepare } from '../template';
import { exit } from 'process';
import { registerAccountPost } from '../controllers/accounts.controller';

let router: express.Router = express.Router();

router.post('/register', registerAccountPost);

export default router;