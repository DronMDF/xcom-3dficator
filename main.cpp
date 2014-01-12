
#include <iostream>
#include <array>
#include <boost/lexical_cast.hpp>
#include <boost/geometry.hpp>
#include <png++/png.hpp>
#include "XCOMContainer.h"

// Ждем релиза upp11 3.0 чтобы убрать этот костыль.
#undef assert
#include "upp11.h"

using namespace std;
using namespace boost::geometry;
using boost::lexical_cast;

// Вызываться будет примерно так:
// xcom-3dficator -w32 -h48 UNITS/XCOM_0 32 33 34 35 36 37 38 39 > unarmored-male-head-torso.json

// В результате вываливаем список пикселов
// { points: [
//	{coord: {x: 15, y: 15, z: 17}, color: {r: 15, g: 36, b: 22}}
//	etc
// ]}

vector<array<uint8_t, 3>> loadPalette(const string &filename, int offset, int ncolors)
{
	ifstream pf(filename.c_str(), ios::in | ios::binary);
	if (!pf) {
		throw runtime_error(filename + " not found");
	}

	// Move pointer to proper pallete
	pf.seekg(offset, ios::beg);

	vector<array<uint8_t, 3>> palette;
	for (int i = 0; i < ncolors; ++i) {
		array<uint8_t, 3> entry;
		pf.read(reinterpret_cast<char *>(&entry[0]), 3);
		entry[0] *= 4;
		entry[1] *= 4;
		entry[2] *= 4;
		palette.push_back(entry);
	}

	return palette;
}

void pngsave(const string &filename, const vector<uint8_t> bitmap, int width, int height,
	     const vector<array<uint8_t, 3>> &palette)
{
	png::image<png::rgba_pixel> image(width, height);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const auto code = bitmap[y * width + x];
			const auto color = palette[code];
			image.set_pixel(x, y,
				png::rgba_pixel(color[0], color[1], color[2], (code == 0) ? 0 : 255));
		}
	}

	image.write(filename);
}

typedef model::point<double, 3, cs::cartesian> point3d;

class rotate_transformer_z : public strategy::transform::ublas_transformer<point3d, point3d, 3, 3>
{
public :
	rotate_transformer_z(const double &angle)
		: ublas_transformer<point3d, point3d, 3, 3>(
			 cos(angle), sin(angle), 0, 0,
			-sin(angle), cos(angle), 0, 0,
			 0,	     0,		 1, 0,
			 0,          0,          0, 1)
	{
	}
};



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

	const auto palette = loadPalette("GEODATA/PALETTES.DAT", 774, 256);
	int n = 0;
	for (auto f : facings) {
		pngsave(lexical_cast<string>(n++) + ".png", f, 32, 48, palette);
		cout << "facing size: " << f.size() << endl;
	}

	point3d one_vector(1, 0, 0);
	point3d facing2;
	rotate_transformer_z(-35.264 * math::d2r).apply(one_vector, facing2);

	cout << get<0>(facing2) << " " << get<1>(facing2) << " " << get<2>(facing2) << endl;

	return 0;
}

