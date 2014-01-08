
#include "XCOMContainer.h"

using namespace std;

shared_ptr<XCOMContainer> XCOMContainer::create(const string &/*filename*/, int /*width*/, int /*height*/)
{
	return make_shared<XCOMContainer>();
}

vector<uint8_t> XCOMContainer::getBitmap(int /*index*/) const
{
	return {};
}
