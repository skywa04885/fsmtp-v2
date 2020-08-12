/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include "SSLContext.src.h";

using namespace FSMTP::Sockets;

SSLContext::SSLContext(const char *p_KeyPath, const char *p_CertPath) noexcept:
  p_KeyPath(p_KeyPath), p_CertPath(p_CertPath)
{}

void SSLContext::read(const SSL_METHOD *method)
{
  SSL_CTX *&ctx= this->p_SSLCtx;

  ctx = SSL_CTX_new(method);
  if (!ctx)
    throw runtime_error(EXCEPT_DEBUG("Could not create SSL Context"));
  
  if (SSL_CTX_use_PrivateKey_file(ctx, this->p_KeyPath, SSL_FILETYPE_PEM) <= 0)
  {

  }
}
