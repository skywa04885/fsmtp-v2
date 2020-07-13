import express from 'express';
import path from 'path';
import { prepare } from '../template';
import registerLang from '../lang/accounts/register';
import loginLang from '../lang/accounts/login';
import aboutLang from '../lang/accounts/about';
import { exit } from 'process';
import { registerAccountPost } from '../controllers/accounts.controller';

let router: express.Router = express.Router();

router.get('/send', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Gets the language object
  let lang = Object.assign(registerLang.english, aboutLang.english);

  // Renders the page
  res.render('fsmtp/send.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'register,software,fannst,account,create,email'
  }, lang));
});

router.post('/register', registerAccountPost);

export default router;