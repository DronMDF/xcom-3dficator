
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "XCOMContainer.h"

// Ждем релиза upp11 3.0 чтобы убрать этот костыль.
#undef assert
#include "upp11.h"

using namespace std;
using namespace boost;

// Вызываться будет примерно так:
// xcom-3dficator -w32 -h48 UNITS/XCOM_0 32 33 34 35 36 37 38 39 > unarmored-male-head-torso.json

// В результате вываливаем список пикселов
// { points: [
//	{coord: {x: 15, y: 15, z: 17}, color: {r: 15, g: 36, b: 22}}
//	etc
// ]}

int main(int /*argc*/, char **argv)
{
	// TODO: Нужно сделать нулевой verbosity level
	UP_RUN();

	const auto container = XCOMContainer::create(argv[1], 32, 48);
	const auto facings = {
		container->getBitmap(lexical_cast<int>(argv[2])),
		container->getBitmap(lexical_cast<int>(argv[3])),
		container->getBitmap(lexical_cast<int>(argv[4])),
		container->getBitmap(lexical_cast<int>(argv[5])),
		container->getBitmap(lexical_cast<int>(argv[6])),
		container->getBitmap(lexical_cast<int>(argv[7])),
		container->getBitmap(lexical_cast<int>(argv[8])),
		container->getBitmap(lexical_cast<int>(argv[9]))
	};

	for (auto f : facings) {
		cout << "facing size: " << f.size() << endl;
	}

	return 0;
}

