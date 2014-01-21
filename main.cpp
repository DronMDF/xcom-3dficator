
#include <iostream>
#include <array>
#include <iomanip>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <png++/png.hpp>
#include "upp11.h"
#include "XCOMContainer.h"

using namespace std;
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

typedef tuple<double, double, double> point3d;

double d2r(double angle)
{
	return angle * M_PI / 180;
}

point3d rotateOverY(const point3d &point, double angle)
{
	return point3d(get<0>(point) * cos(angle) - get<2>(point) * sin(angle),
		       get<1>(point),
		       get<0>(point) * sin(angle) + get<2>(point) * cos(angle));
}

point3d rotateOverX(const point3d &point, double angle)
{
	return point3d(get<0>(point),
		       get<2>(point) * sin(angle) + get<1>(point) * cos(angle),
		       get<2>(point) * cos(angle) - get<1>(point) * sin(angle));
}

bool isVisiblePoint(const point3d &point, const vector<uint8_t> facings[8])
{
	const int yangle[8] = { 135, 90, 45, 0, -45, -90, -135, -180 };
	for (int f = 0; f < 8; f++) {
		const point3d face = rotateOverY(point, d2r(yangle[f]));
		const point3d dim = rotateOverX(face, d2r(35.264));
		const int xp = get<0>(dim) + 16;
		const int yp = get<1>(dim) * 200/240;	// аспект для VGA
		if (yp < 0 || yp > 47 || xp < 0 || xp > 32 || facings[f][(38 - yp) * 32 + xp] == 0) {
			return false;
		}
	}
	return true;
}

vector<point3d> generatePoints(const vector<uint8_t> facings[8])
{
	vector<point3d> result;
	for (double ym = 0.25; ym < 64; ym += .5) {
		for (double xm = -15.75; xm < 16; xm += .5) {
			for (double zm = -15.75; zm < 16; zm += .5) {
				const point3d current(xm, ym, zm);
				if (isVisiblePoint(current, facings)) {
					//cout << format("point %1%,%2%,%3%") % xm % ym % zm << endl;
					result.push_back(current);
				}
			}
		}
	}
	return result;
}

vector<pair<point3d, uint8_t>> coloredPoints(const vector<point3d> &points, const vector<uint8_t> facings[8])
{
	const int yangle[8] = { 135, 90, 45, 0, -45, -90, -135, -180 };
	vector<uint8_t> color(points.size(), 0);
	for (int f = 0; f < 8; f++) {
		// Вращаем сразу все на фейсинг
		vector<point3d> rotated(points.size());
		for (unsigned i = 0; i < points.size(); i++) {
			const point3d face = rotateOverY(points[i], d2r(yangle[f]));
			rotated[i] = rotateOverX(face, d2r(35.264));
		}

		// сортируем по квадратикам и расцвечиваем только верхние
		for (int y = 0; y < 48; y++) {
			for (int x = 0; x < 32; x++) {

				vector<pair<double, int>> deeps;
				for (unsigned i = 0; i < rotated.size(); i++) {
					const int xp = get<0>(rotated[i]) + 16;
					const int yp = 38 - get<1>(rotated[i]) * 200/240;	// аспект для VGA
					if (xp == x && yp == y) {
						deeps.push_back(make_pair(get<2>(rotated[i]), i));
					}
				}

				sort(deeps.begin(), deeps.end());
				for (unsigned i = 0; i < min(deeps.size(), 5U); i++) {
					// одну-пять точки маркируем
					color[deeps[i].second] = facings[f][y * 32 + x];
				}
			}
		}
	}

	vector<pair<point3d, uint8_t>> result;
	for (unsigned i = 0; i < points.size(); i++) {
		if (color[i] != 0) {
			result.push_back(make_pair(points[i], color[i]));
		}
	}
	return result;
}

int main(int argc, char **argv)
{
	// TODO: Нужно сделать нулевой verbosity level
	if (!UP_RUN()) {
		return -1;
	}

	string varname;
	while (true) {
		int opt = getopt(argc, argv, "v:");
		if (opt == -1) { break; }
		if (opt == 'v') { varname = optarg; }
	};

	const auto container = XCOMContainer::create(argv[optind], 32, 48);
	const vector<uint8_t> facings[8] = {
		container->getBitmap(lexical_cast<int>(argv[optind + 1])),
		container->getBitmap(lexical_cast<int>(argv[optind + 2])),
		container->getBitmap(lexical_cast<int>(argv[optind + 3])),
		container->getBitmap(lexical_cast<int>(argv[optind + 4])),
		container->getBitmap(lexical_cast<int>(argv[optind + 5])),
		container->getBitmap(lexical_cast<int>(argv[optind + 6])),
		container->getBitmap(lexical_cast<int>(argv[optind + 7])),
		container->getBitmap(lexical_cast<int>(argv[optind + 8]))
	};

	const vector<point3d> obj_points = generatePoints(facings);

	const vector<pair<point3d, uint8_t>> obj_colored_points = coloredPoints(obj_points,facings);

	const auto palette = loadPalette("GEODATA/PALETTES.DAT", 774, 256);

	cout << "var " << varname << " = {" << endl;
	cout << "\tpoints: [ ";
	for (const auto cp: obj_colored_points) {
		cout << format("%1%, %2%, %3%, ") % get<0>(cp.first) % get<1>(cp.first) % get<2>(cp.first);
	}
	cout << "]," << endl;
	cout << "\tcolors: [ ";
	for (const auto cp: obj_colored_points) {
		const auto color = palette[cp.second];
		cout << format("%1%, %2%, %3%, ") % (color[0] / 256.) % (color[1] / 256.) % (color[2] / 256.);
	}
	cout << "]," << endl;
	cout << "\tpoints_count: " << obj_colored_points.size() << endl;
	cout << "};" << endl;
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
	const point3d given(1, 0, 0);
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(45)), point3d(.707107, 0, .707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(90)), point3d(0, 0, 1));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(135)), point3d(-.707107, 0, .707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(180)), point3d(-1, 0, 0));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(225)), point3d(-.707107, 0, -.707107));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(-90)), point3d(0, 0, -1));
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverY(given, d2r(-45)), point3d(.707107, 0, -.707107));
}

UP_TEST(rotationOverXShouldCorrect)
{
	const point3d given(0, 0, -1);
	CUSTOM_ASSERT_POINT_EQUAL(rotateOverX(given, d2r(35.264)), point3d(0, -.577345, -.816501));
}
