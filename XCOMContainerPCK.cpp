
#include "XCOMContainerPCK.h"
#include <fstream>
#include <iterator>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using boost::adaptors::reverse;

XCOMContainerPCK::XCOMContainerPCK(const string &filename, int width, int height)
	: width(width), height(height)
{
	vector<uint16_t> pck_tab;
	ifstream ftab((filename + ".TAB").c_str(), ios::in | ios::binary);
	uint16_t tabv;
	while(ftab.read(reinterpret_cast<char *>(&tabv), 2)) {
		pck_tab.push_back(tabv);
	}

	vector<uint8_t> pck_raw;
	ifstream fpck((filename + ".PCK").c_str(), ios::in | ios::binary);
	fpck >> noskipws;
	copy(istream_iterator<uint8_t>(fpck), istream_iterator<uint8_t>(), back_inserter(pck_raw));

	int last = pck_raw.size();
	for (const uint16_t &offset : reverse(pck_tab)) {
		const vector<uint8_t> pck_item(pck_raw.begin() + offset, pck_raw.begin() + last);
		pck_data.insert(pck_data.begin(), pck_item);
		last = offset;
	}
}

std::vector<uint8_t> XCOMContainerPCK::getBitmap(int index) const
{
	auto cursor = pck_data[index].begin();

	vector<uint8_t> bitmap(width * height, 0);
	size_t bp = *cursor++ * width;
	bool eof = false;

	while (cursor != pck_data[index].end()) {
		assert(!eof);
		if (*cursor == 0xfe) {
			cursor++;
			bp += *cursor++;
			continue;
		}

		if (*cursor == 0xff) {
			cursor++;
			eof = true;
			continue;
		}

		bitmap[bp++] = *cursor++;
	}

	return bitmap;
}
