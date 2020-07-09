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

#include "SMTPSpamDetection.src.h"

namespace FSMTP::Server::SpamDetection
{
	bool checkSpamhaus(std::string ip)
	{
		std::vector<std::string> segments = {};
		std::stringstream stream(ip);
		std::string token;
		std::stringstream res;

		// Reverses the IP address, so
		// - we can perform the lookup
		while (std::getline(stream, token, '.'))
			segments.push_back(token);

		std::reverse(segments.begin(), segments.end());
		ip.clear();
		std::copy(
			segments.begin(),
			segments.end(),
			std::ostream_iterator<std::string>(res, ".")
		);
		ip = res.str();

		// Performs the spamhaus lookup
	}
}