
#pragma once
#include "XCOMContainer.h"

class XCOMContainerPCK : public XCOMContainer
{
public:
	XCOMContainerPCK(const std::string &filename, int width, int height);
	virtual std::vector<uint8_t> getBitmap(int index) const override;

private:
	int width;
	int height;
	std::vector<std::vector<uint8_t>> pck_data;
};
