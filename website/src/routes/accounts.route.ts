import { Router } from 'express';
import { register } from '../controllers/register.controller';

let router: Router = Router();

router.post('/register', register);

export default router;