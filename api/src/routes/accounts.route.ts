import express from 'express';
import path from 'path';
import { prepare } from '../template';
import registerLang from '../lang/accounts/register';
import loginLang from '../lang/accounts/login';
import aboutLang from '../lang/accounts/about';

let router: express.Router = express.Router();

router.get('/register', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Gets the language object
  let lang = Object.assign(registerLang.english, aboutLang.english);

  // Renders the page
  res.render('accounts/register.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'register,software,fannst,account,create,email'
  }, lang));
});

router.get('/login', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  // Gets the language object
  let lang = Object.assign(loginLang.english, aboutLang.english);

  // Renders the page
  res.render('accounts/login.view.ejs', prepare({
    title: lang.title,
    description: lang.subtitle,
    keywords: 'login,software,fannst,account,use,email'
  }, lang));
});

export default router;