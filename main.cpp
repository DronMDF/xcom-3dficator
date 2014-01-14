
#include <iostream>
#include <array>
#include <boost/lexical_cast.hpp>
#include <boost/geometry.hpp>
#include <boost/format.hpp>
#include <png++/png.hpp>
#include "upp11.h"
#include "XCOMContainer.h"

using namespace std;
using namespace boost::geometry;
using boost::lexical_cast;
using boost::format;

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

point3d rotateOverZ(const point3d &point, double angle)
{
	class rotator : public strategy::transform::ublas_transformer<point3d, point3d, 3, 3> {
	public :
		rotator(const double &angle)
			: ublas_transformer<point3d, point3d, 3, 3>(
				 cos(angle), sin(angle), 0, 0,
				-sin(angle), cos(angle), 0, 0,
				 0,	     0,		 1, 0,
				 0,          0,          0, 1)
		{
		}
	};
	point3d result;
	rotator(angle).apply(point, result);
	return result;
}

point3d rotateOverY(const point3d &point, double angle)
{
	return point3d(get<0>(point) * cos(angle) - get<2>(point) * sin(angle),
		       get<1>(point),
		       get<0>(point) * sin(angle) + get<2>(point) * cos(angle));
}

point3d rotateOverX(const point3d &point, double angle)
{
	class rotator : public strategy::transform::ublas_transformer<point3d, point3d, 3, 3> {
	public :
		rotator(const double &angle)
			: ublas_transformer<point3d, point3d, 3, 3>(
				1, 0, 0, 0,
				0, cos(angle), sin(angle), 0,
				0, -sin(angle), cos(angle), 0,
				0,          0,           0, 1)
		{
		}
	};
	point3d result;
	rotator(angle).apply(point, result);
	return result;
}

int main(int /*argc*/, char **argv)
{
	// TODO: Нужно сделать нулевой verbosity level
	if (!UP_RUN()) {
		return -1;
	}

	const auto container = XCOMContainer::create(argv[1], 32, 48);
	const vector<uint8_t> facings[8] = {
		container->getBitmap(lexical_cast<int>(argv[2])),
		container->getBitmap(lexical_cast<int>(argv[3])),
		container->getBitmap(lexical_cast<int>(argv[4])),
		container->getBitmap(lexical_cast<int>(argv[5])),
		container->getBitmap(lexical_cast<int>(argv[6])),
		container->getBitmap(lexical_cast<int>(argv[7])),
		container->getBitmap(lexical_cast<int>(argv[8])),
		container->getBitmap(lexical_cast<int>(argv[9]))
	};

//	const auto palette = loadPalette("GEODATA/PALETTES.DAT", 774, 256);
//	int n = 0;
//	for (auto f : facings) {
//		pngsave(lexical_cast<string>(n++) + ".png", f, 32, 48, palette);
//		cout << "facing size: " << f.size() << endl;
//	}

//	const point3d one_vector(1, 0, 0);
//	const point3d facing_vec = rotateOverZ(one_vector, -35.264 * math::d2r);
//	const point3d facings_vec[8] = {
//		rotateOverY(facing_vec, 45 * math::d2r),
//		facing_vec,
//		rotateOverY(facing_vec, -45 * math::d2r),
//		rotateOverY(facing_vec, -90 * math::d2r),
//		rotateOverY(facing_vec, -135 * math::d2r),
//		rotateOverY(facing_vec, -180 * math::d2r),
//		rotateOverY(facing_vec, -225 * math::d2r),
//		rotateOverY(facing_vec, -270 * math::d2r)
//	};

	const int yangle[8] = { 135, 90, 45, 0, -45, -90, -135, -180 };

	int obj[32][32][64];

	for (int xm = 0; xm < 32; xm++) {
		for (int zm = 0; zm < 32; zm++) {
			for (int ym = 0; ym < 64; ym++) {
				const point3d current(xm - 16 + .5, ym + .5, zm - 16 + .5);
				obj[xm][zm][ym] = 0x100;
				//cout << format("point %1%,%2%,%3%") % xm % zm % ym << endl;
				for (int f = 0; f < 8; f++) {
					const point3d face = rotateOverY(current, yangle[f] * math::d2r);
					const point3d dim = rotateOverX(face, 35.264 * math::d2r);
					const int xp = get<0>(dim) + 16;
					const int yp = get<1>(dim) * 200/240;	// аспект для VGA
					//cout << format("facing %1% map to %2%,%3%") % f % xp % yp << endl;
					if (yp < 0 || yp > 47 || xp < 0 || xp > 32 ||
						facings[f][(38 - yp) * 32 + xp] == 0)
					{
						obj[xm][zm][ym] = 0;
						break;
					}
				}
			}
		}
	}

	for (int ym = 0; ym < 64; ym++) {
		for (int xm = 0; xm < 32; xm++) {
			cout << "> ";
			for (int zm = 0; zm < 32; zm++) {
				cout << (obj[xm][zm][ym] != 0 ? "[]" : "  ");
			}
			cout << endl;
		}
		cout << endl;
	}

	return 0;
}


void CUSTOM_ASSERT_POINT_EQUAL(const point3d &a, const point3d &b)
{
	UP_ASSERT(fabs(get<0>(a) - get<0>(b)) < 1e-6);
	UP_ASSERT(fabs(get<1>(a) - get<1>(b)) < 1e-6);
	UP_ASSERT(fabs(get<2>(a) - get<2>(b)) < 1e-6);
}

UP_TEST(rotationOverYShouldCorrect)
{
	point3d given(1, 0, 0);
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, 45 * math::d2r), point3d(.707107, 0, .707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, 90 * math::d2r), point3d(0, 0, 1));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, 135 * math::d2r), point3d(-.707107, 0, .707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, 180 * math::d2r), point3d(-1, 0, 0));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, 225 * math::d2r), point3d(-.707107, 0, -.707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, -90 * math::d2r), point3d(0, 0, -1));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, -45 * math::d2r), point3d(.707107, 0, -.707107));
}
