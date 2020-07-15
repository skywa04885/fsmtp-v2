import express from 'express';
import path from 'path';
import { prepare } from '../template';
import { exit } from 'process';
import { registerAccountPost } from '../controllers/accounts.controller';

let router: express.Router = express.Router();

router.get('/register', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Renders the page
  res.render('accounts/register.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'register,software,fannst,account,create,email'
  }));
});

router.get('/login', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Renders the page
  res.render('accounts/login.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'login,software,fannst,account,use,email'
  }));
});

router.post('/register', registerAccountPost);

export default router;