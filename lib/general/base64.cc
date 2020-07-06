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

// ==================================
// [OLD CODE] It sucks !
// ==================================

#include "base64.h"

namespace FSMTP::Base64
{
  template <typename T>
  void _printBits(T bin, int start_pos)
  {
    std::string res;
    unsigned int bcp = bin;
    for (std::size_t i = start_pos; i < sizeof(bin) * 8; i++)
    {
      if (bcp & 0x01) res += '1';
      else res += '0';

      bcp >>= 1;
    }

    std::reverse(res.begin(), res.end());
    std::cout << " - " << (int)bin << ": " << "\033[31m0b\033[32m" << res << "\033[0m (" << sizeof(bin) * 8 - start_pos<< " BITS)" << std::endl;
  }

  const char _dictionary[] = {
    'A','B','C','D','E','F','G','H','I','J',
    'K','L','M','N','O','P','Q','R','S','T',
    'U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j',
    'k','l','m','n','o','p','q','r','s','t',
    'u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9',
    '+','/'
  };

  unsigned char _reverseDict(const char& c)
  {
    if (c >= 'A' && c <= 'Z')
    {
      switch (c)
      {
        case 'A': { return 0; }
        case 'B': { return 1; }
        case 'C': { return 2; }
        case 'D': { return 3; }
        case 'E': { return 4; }
        case 'F': { return 5; }
        case 'G': { return 6; }
        case 'H': { return 7; }
        case 'I': { return 8; }
        case 'J': { return 9; }
        case 'K': { return 10; }
        case 'L': { return 11; }
        case 'M': { return 12; }
        case 'N': { return 13; }
        case 'O': { return 14; }
        case 'P': { return 15; }
        case 'Q': { return 16; }
        case 'R': { return 17; }
        case 'S': { return 18; }
        case 'T': { return 19; }
        case 'U': { return 20; }
        case 'V': { return 21; }
        case 'W': { return 22; }
        case 'X': { return 23; }
        case 'Y': { return 24; }
        case 'Z': { return 25; }
      }
    } else if (c >= 'a' && c <= 'z')
    {
      switch (c)
      {
        case 'a': { return 26; }
        case 'b': { return 27; }
        case 'c': { return 28; }
        case 'd': { return 29; }
        case 'e': { return 30; }
        case 'f': { return 31; }
        case 'g': { return 32; }
        case 'h': { return 33; }
        case 'i': { return 34; }
        case 'j': { return 35; }
        case 'k': { return 36; }
        case 'l': { return 37; }
        case 'm': { return 38; }
        case 'n': { return 39; }
        case 'o': { return 40; }
        case 'p': { return 41; }
        case 'q': { return 42; }
        case 'r': { return 43; }
        case 's': { return 44; }
        case 't': { return 45; }
        case 'u': { return 46; }
        case 'v': { return 47; }
        case 'w': { return 48; }
        case 'x': { return 49; }
        case 'y': { return 50; }
        case 'z': { return 51; }
      }
    } else if (c >= '0' && c <= '9')
    {
      switch (c)
      {
        case '0': { return 52; }
        case '1': { return 53; }
        case '2': { return 54; }
        case '3': { return 55; }
        case '4': { return 56; }
        case '5': { return 57; }
        case '6': { return 58; }
        case '7': { return 59; }
        case '8': { return 60; }
        case '9': { return 61; }
      }
    } else
    {
      switch (c)
      {
        case '+': { return 62; }
        case '/': { return 63; }
        case '=': { return 99; }
      }
    }
  }

  const char *encode(const char *raw)
  {
    std::size_t buffer_index = 0;
    char *result = reinterpret_cast<char *>(malloc(1));
    result[0] = '\0';
    std::size_t resultSize = 1;
    unsigned char buffer[3] = {0, 0, 0};

    unsigned char bf_sa;
    unsigned char bf_sb;
    unsigned char bf_sc;
    unsigned char bf_sd;
    unsigned char temp;

    /**
     * Encode pairs of three, as long as possible
     */

    for (const char *c = &raw[0]; *c != '\0'; c++)
    {
      buffer[buffer_index] = *c;
      buffer_index++;

      // Checks if it should encode
      if (buffer_index >= 3)
      { // Buffer is at max
        #ifdef DEBUG
        std::cout << "Starting encoding octets: [\033[34m" << (int)buffer[0] << "\033[0m, \033[34m"
            << (int)buffer[1] << "\033[0m, \033[34m" << (int)buffer[2]
            << "\033[0m], buffer_index: \033[34m" << buffer_index << "\033[0m" << std::endl;
        #endif

        /**
         * First 6 bits
         */

        bf_sa = buffer[0] & 0b11111100;     // Selects      xxxx xx--
        bf_sa >>= 2;                        // Moves        xxxx xx-- => --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SA, value: " << static_cast<int>(bf_sa) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sa, 2);
        #endif

        /**
         * Second 6 bits
         */

        // Prepares the last 2 bits of octet buffer[0]
        bf_sb = buffer[0] & 0b00000011;         // Selects      ---- --xx
        bf_sb <<= 4;                            // Moves        ---- --xx => --xx ----
        // Adds the first 4 bits of octet buffer[1]
        temp = buffer[1] & 0b11110000;          // Selects      xxxx ----
        temp >>= 4;                             // Moves        xxxx ---- => ---- xxxx
        bf_sb |= temp;                          // Merges at    --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SB, value: " << static_cast<int>(bf_sb) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sb, 2);
        #endif

        /**
         * Third 6 bits
         */

        // Gets the last 4 bits of octet buffer[1]
        bf_sc = buffer[1] & 0b00001111;         // Selects      ---- xxxx
        bf_sc <<= 2;                            // Moves        ---- xxxx => --xx xx--
        // Gets the first 2 bits of octet buffer[2]
        temp = buffer[2] & 0b11000000;          // Selects      xx-- ----
        temp >>= 6;                             // Moves        xx-- ---- => ---- --xx
        bf_sc |= temp;                          // Merges at    --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SC, value: " << static_cast<int>(bf_sc) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sc, 2);
        #endif

        /**
         * Fourth 6 bits
         */

        // Gets the last 6 bits of octet buffer[2]
        bf_sd = buffer[2] & 0b00111111;         // Selects      --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SD, value: " << static_cast<int>(bf_sd) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sd, 2);
        #endif

        /**
         * Adds them to the result string
         */

        char *t = reinterpret_cast<char *>(malloc(5));
        t[4] = '\0';

        t[0] = _dictionary[bf_sa];
        t[1] = _dictionary[bf_sb];
        t[2] = _dictionary[bf_sc];
        t[3] = _dictionary[bf_sd];

        resultSize += 4;
        result = reinterpret_cast<char *>(realloc(&result[0], resultSize));
        strcat(&result[0], &t[0]);
        delete t;

        #ifdef DEBUG
        std::cout << "Finished encoding for buffer_index: \033[34m" << buffer_index << "\033[0m" << std::endl;
        #endif

        /**
         * Cleans the memory
         */

        memset(buffer, 0, sizeof(buffer));      // Clears the buffer
        buffer_index = 0;                          // Sets the buffer index to zero
      }
    }

    /**
     * Checks if there are 1 or 2 chars left for encoding, and encodes them
     */

    if (buffer_index > 0)
    {
      if (buffer_index == 1)
      { // To be encoded left
        #ifdef DEBUG
        std::cout << "Detected single octet to be encoded" << std::endl;
        #endif


        /**
         * First 6 bits
         */

        bf_sa = buffer[0] & 0b11111100;     // Selects      xxxx xx--
        bf_sa >>= 2;                        // Moves        xxxx xx-- => --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SA, value: " << static_cast<int>(bf_sa) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sa, 2);
        #endif

        /**
         * Second 6 bits
         */

        bf_sb = buffer[0] & 0b00000011;    // Selects      ---- --xx
        bf_sb <<= 4;                       // Moves        ---- --xx => --xx ----

        #ifdef DEBUG
        std::cout << "Encoded BF_SB, value: " << static_cast<int>(bf_sb) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sb, 2);
        #endif

        /**
         * Adds them to the result string
         */

        char *t = reinterpret_cast<char *>(malloc(5));
        t[4] = '\0';

        t[0] = _dictionary[bf_sa];
        t[1] = _dictionary[bf_sb];
        t[2] = '=';
        t[3] = '=';

        resultSize += 4;
        result = reinterpret_cast<char *>(realloc(&result[0], resultSize));
        strcat(&result[0], &t[0]);
        delete t;

        #ifdef DEBUG
        std::cout << "Finished final octet" << std::endl;
        #endif
      } else if (buffer_index == 2)
      { // Two to be encoded left
        #ifdef DEBUG
        std::cout << "Detected two octets to be endcoded" << std::endl;
        #endif

        /**
         * First 6 bits
         */

        // Gets the first 6 bits
        bf_sa = buffer[0] & 0b11111100;         // Selects      xxxx xx--
        bf_sa >>= 2;                            // Moves        xxxx xx-- => --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SA, value: " << static_cast<int>(bf_sa) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sa, 2);
        #endif

        /**
         * Second 6 bits
         */

        // Gets the last 2 bits of octet buffer[0]
        bf_sb = buffer[0] & 0b00000011;         // Selects      ---- --xx
        bf_sb <<= 4;                            // Moves        ---- --xx => --xx ----
        // Gets the first 4 bits of octet buffer[1]
        temp = buffer[1] & 0b11110000;          // Selects      xxxx ----
        temp >>= 4;                             // Moves        xxxx ---- => ---- xxxx
        // Merges bf_sb and temp
        bf_sb |= temp;                          // Mergex at    --xx xxxx

        #ifdef DEBUG
        std::cout << "Encoded BF_SB, value: " << static_cast<int>(bf_sb) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sb, 2);
        #endif

        /**
         * Third 6 bits
         */

        // Selects the last 4 bytes of octet buffer[1]
        bf_sc = buffer[1] & 0b00001111;         // Selects      ---- xxxx
        bf_sc <<= 2;                            // Moves        ---- xxxx => --xx xx--

        #ifdef DEBUG
        std::cout << "Encoded BF_SC, value: " << static_cast<int>(bf_sc) << ", binary: " << std::endl;
        _printBits<unsigned char>(bf_sc, 2);
        #endif

        /**
         * Appends the result
         */

        char *t = reinterpret_cast<char *>(malloc(5));
        t[4] = '\0';

        t[0] = _dictionary[bf_sa];
        t[1] = _dictionary[bf_sb];
        t[2] = _dictionary[bf_sc];
        t[3] = '=';

        resultSize += 4;
        result = reinterpret_cast<char *>(realloc(&result[0], resultSize));
        strcat(&result[0], &t[0]);
        delete t;

        #ifdef DEBUG
        std::cout << "Finished final two octets" << std::endl;
        #endif
      } else std::exit(-1);
    }

    /**
     * Returns the result
     */

    return reinterpret_cast<const char *>(result);
  }

  const char *decode(const char *encoded)
  {
    char *result = reinterpret_cast<char *>(malloc(1));
    result[0] = '\0';
    std::size_t buffer_index = 0;
    unsigned char ch_a;
    unsigned char ch_b;
    unsigned char ch_c;
    unsigned char temp;
    unsigned char buffer[4];

    /**
     * Loops over all the chars
     */

    for (char *c = &result[0]; *c != '\0'; c++)
    {
      buffer[buffer_index] = _reverseDict(*c);
      buffer_index++;

      // Checks if the buffer is full
      if (buffer_index >= 4)
      {
        /**
         * First octet
         */

        // Gets the first 6 bits
        ch_a = buffer[0] & 0b00111111;              // Select       --xx xxxx
        ch_a <<= 2;                                 // Moves        --xx xxxx => xxxx xx--
        // Gets the first 2 bits of buffer[1]
        temp = buffer[1] & 0b00110000;              // Selects      --xx ----
        temp >>= 4;                                 // Moves        --xx ---- => ---- --xx
        ch_a |= temp;                               // Merges at    xxxx xxxx

        #ifdef DEBUG
        std::cout << "Decoded CH_A, value: " << ch_a << ", binary: " << std::endl;
        _printBits<unsigned char>(ch_a, 0);
        #endif

        result += ch_a;

        /**
         * Second octet
         */

        // Checks if it reached an =
        if (buffer[2] == 99) continue;

        // Gets the last 4 bits of buffer[1]
        ch_b = buffer[1] & 0b00001111;              // Selects      ---- xxxx
        ch_b <<= 4;                                 // Moves        ---- xxxx => xxxx ----
        // Gets the first 4 bits of buffer[2]
        temp = buffer[2] & 0b00111100;              // Selects      xxxx ----
        temp >>= 2;                                 // Moves        xxxx ---- => ---- xxxx
        ch_b |= temp;                               // Merges at    xxxx xxxx

        #ifdef DEBUG
        std::cout << "Decoded CH_B, value: " << ch_b << ", binary: " << std::endl;
        _printBits<unsigned char>(ch_b, 0);
        #endif

        result += ch_b;

        /**
         * Third octet
         */

        // Checks if it reached an =
        if (buffer[3] == 99) continue;

        // Gets the last two bits of buffer[2]
        ch_c = buffer[2] & 0b00000011;              // Selects      ---- --xx
        ch_c <<= 6;                                 // Moves        ---- --xx => xx-- ----
        // Gets the last 6 bits of buffer[3]
        temp = buffer[3] & 0b00111111;              // Selects      --xx xxxx
        ch_c |= temp;                               // Merges at    xxxx xxxx

        #ifdef DEBUG
        std::cout << "Decoded CH_C, value: " << ch_c << ", binary: " << std::endl;
        _printBits<unsigned char>(ch_c, 0);
        #endif

        memset(buffer, 0, sizeof(buffer));
        buffer_index = 0;

        result += ch_c;
      }
    }

    /**
     * Returns the result
     */

    return result;
  }
}