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

#pragma once

#include "../default.h"
#include "hex.src.h"

namespace FSMTP::Encoding
{
	/**
	 * Encodes an vector range to quoted printable
	 */
	string decodeQuotedPrintableRange(strvec_it from, strvec_it to);
	
	/**
	 * Decodes an line of quoted printable data
	 */
	string decodeQuotedPrintableLine(const string &line);

	string decodeQuotedPrintable(const string &raw);
	string encodeQuotedPrintable(const string &raw);
	string escapeHTML(const string &raw);
}