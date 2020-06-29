#include "main.h"

int main(const int argc, const char **argv)
{
	int32_t opts = 0x0;

	opts |= _SERVER_OPT_ENABLE_AUTH;
	opts |= _SERVER_OPT_ENABLE_TLS;

	SMTPServer server(465, true, opts);

	std::cin.get();
	server.shutdown();

	return 0;
}