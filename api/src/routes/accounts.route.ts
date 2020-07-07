import express from 'express';
import path from 'path';

let router: express.Router = express.Router();

router.get('/register', (
  req: express.Request,
  res: express.Response,
  next: express.NextFunction
) => {
  res.render('accounts/register.view.ejs');
});

export default router;