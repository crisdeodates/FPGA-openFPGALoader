// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2019 Gwenhael Goavec-Merou <gwenhael.goavec-merou@trabucayre.com>
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>

#include "display.hpp"

#include "configBitstreamParser.hpp"

using namespace std;

ConfigBitstreamParser::ConfigBitstreamParser(const string &filename, int mode,
			bool verbose): _filename(filename), _bit_length(0),
			_file_size(0), _verbose(verbose),
			_bit_data(), _raw_data(), _hdr()
{
	(void) mode;
	if (!filename.empty()) {
		FILE *_fd = fopen(filename.c_str(), "rb");
		if (!_fd)
			throw std::runtime_error("Error: fail to open " + _filename);

		fseek(_fd, 0, SEEK_END);
		_file_size = ftell(_fd);
		fseek(_fd, 0, SEEK_SET);

		_raw_data.resize(_file_size);
		_bit_data.reserve(_file_size);

		int ret = fread((char *)&_raw_data[0], sizeof(char), _file_size, _fd);
		if (ret != _file_size)
			throw std::runtime_error("Error: fail to read " + _filename);
		fclose(_fd);
	} else if (!isatty(fileno(stdin))) {
		_file_size = 0;
		string tmp;
		tmp.resize(4096);
		size_t size;

		do {
			size = fread((char *)&tmp[0], sizeof(char), 4096, stdin);
			_raw_data.append(tmp, 0, size);
			_file_size += size;
		} while (size > 0);
	} else {
		throw std::runtime_error("Error: fail to parse. No filename or pipe\n");
	}
}

ConfigBitstreamParser::~ConfigBitstreamParser()
{
}

string ConfigBitstreamParser::getHeaderVal(string key)
{
	auto val = _hdr.find(key);
	if (val == _hdr.end())
		throw std::runtime_error("Error key " + key + " not found");
	return val->second;
}

void ConfigBitstreamParser::displayHeader()
{
	if (_hdr.empty())
		return;
	cout << "bitstream header infos" << endl;
	for (auto it = _hdr.begin(); it != _hdr.end(); it++) {
		printInfo((*it).first + ": ", false);
		printSuccess((*it).second);
	}
}

static constexpr uint8_t revertByteArr[256] = {
	0, 128, 64, 192, 32, 160, 96, 224, 16, 144,
	80, 208, 48, 176, 112, 240, 8, 136, 72, 200,
	40, 168, 104, 232, 24, 152, 88, 216, 56, 184,
	120, 248, 4, 132, 68, 196, 36, 164, 100, 228,
	20, 148, 84, 212, 52, 180, 116, 244, 12, 140,
	76, 204, 44, 172, 108, 236, 28, 156, 92, 220,
	60, 188, 124, 252, 2, 130, 66, 194, 34, 162,
	98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
	10, 138, 74, 202, 42, 170, 106, 234, 26, 154,
	90, 218, 58, 186, 122, 250, 6, 134, 70, 198,
	38, 166, 102, 230, 22, 150, 86, 214, 54, 182,
	118, 246, 14, 142, 78, 206, 46, 174, 110, 238,
	30, 158, 94, 222, 62, 190, 126, 254, 1, 129,
	65, 193, 33, 161, 97, 225, 17, 145, 81, 209,
	49, 177, 113, 241, 9, 137, 73, 201, 41, 169,
	105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
	5, 133, 69, 197, 37, 165, 101, 229, 21, 149,
	85, 213, 53, 181, 117, 245, 13, 141, 77, 205,
	45, 173, 109, 237, 29, 157, 93, 221, 61, 189,
	125, 253, 3, 131, 67, 195, 35, 163, 99, 227,
	19, 147, 83, 211, 51, 179, 115, 243, 11, 139,
	75, 203, 43, 171, 107, 235, 27, 155, 91, 219,
	59, 187, 123, 251, 7, 135, 71, 199, 39, 167,
	103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
	15, 143, 79, 207, 47, 175, 111, 239, 31, 159,
	95, 223, 63, 191, 127, 255
};

uint8_t ConfigBitstreamParser::reverseByte(uint8_t src)
{
#if 0
	uint8_t dst = 0;
	for (int i=0; i < 8; i++) {
		dst = (dst << 1) | (src & 0x01);
		src >>= 1;
	}
	return dst;
#else
	return revertByteArr[src];
#endif
}
