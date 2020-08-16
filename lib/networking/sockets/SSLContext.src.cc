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

SSLContext::SSLContext() noexcept {}

SSLContext::~SSLContext() noexcept {
	if (this->p_SSLCtx) {
		SSL_CTX_free(this->p_SSLCtx);
	}
}

SSLContext &SSLContext::read(const char *privateKey, const char *cert) {
  SSL_CTX *&ctx= this->p_SSLCtx;
	const SSL_METHOD *method = this->p_SSLMethod;

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    throw runtime_error(EXCEPT_DEBUG("Could not create SSL Context"));
	}

	// Sets the password callback with the user data for the current
	//  ssl context, this will basically read the password from the
	//  current instance, when this is requested by openssl. After
	//  this we set the key and cert file.
	
	SSL_CTX_set_ecdh_auto(ctx, 1);
	SSL_CTX_set_default_passwd_cb(ctx, SSLContext::passwordCallback);
	SSL_CTX_set_default_passwd_cb_userdata(ctx, this);

  if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
  } else if (SSL_CTX_use_PrivateKey_file(ctx, privateKey, SSL_FILETYPE_PEM) <= 0) {
		throw runtime_error(EXCEPT_DEBUG(SSL_STRERROR));
	}

	return *this;
}

SSLContext &SSLContext::password(const char *pass) {
	ifstream in(pass, ios::binary);
	DEFER(in.close());

	in >> this->p_Password;

	return *this;
}

SSLContext &SSLContext::method(const SSL_METHOD *method) {
	this->p_SSLMethod = method;
	return *this;
}

int32_t SSLContext::passwordCallback(
	char *buffer, int32_t size, 
	int32_t rwflag, void *u
) {
	SSLContext *sslCtx = reinterpret_cast<SSLContext *>(u);
	strncpy(buffer, sslCtx->p_Password.c_str(), sslCtx->p_Password.length() + 1);
	return sslCtx->p_Password.length();
}

SSLContext &SSLContext::justCreate() {
	SSL_CTX *&ctx= this->p_SSLCtx;
	const SSL_METHOD *&method = this->p_SSLMethod;

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    throw runtime_error(EXCEPT_DEBUG("Could not create SSL Context"));
	}

	return *this;
}