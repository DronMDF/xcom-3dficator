
#pragma once
#include <memory>
#include <vector>
#include <string>

class XCOMContainer
{
public:
	static std::shared_ptr<XCOMContainer> create(const std::string &filename, int width, int height);

	virtual std::vector<uint8_t> getBitmap(int index) const;
};
