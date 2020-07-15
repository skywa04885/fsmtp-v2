import express from 'express';
import path from 'path';
import { prepare } from '../template';
import { exit } from 'process';
import { registerAccountPost } from '../controllers/accounts.controller';

let router: express.Router = express.Router();

router.get('/send', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Renders the page
  res.render('fsmtp/send.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'register,software,fannst,account,create,email'
  }));
});

router.post('/register', registerAccountPost);

export default router;