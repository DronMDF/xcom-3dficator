
#include "XCOMContainer.h"
#include <boost/filesystem.hpp>
#include "XCOMContainerPCK.h"

using namespace std;
using boost::filesystem::path;
using boost::filesystem::exists;

shared_ptr<XCOMContainer> XCOMContainer::create(const string &filename, int width, int height)
{
	assert(!path(filename).has_extension());
	if (exists(filename + ".TAB") && exists(filename + ".PCK")) {
		return make_shared<XCOMContainerPCK>(filename, width, height);
	}

	return make_shared<XCOMContainer>();
}

vector<uint8_t> XCOMContainer::getBitmap(int /*index*/) const
{
	return {};
}
