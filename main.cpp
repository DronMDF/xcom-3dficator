
#include <iostream>
#include "upp11.h"

using namespace std;

// Вызываться будет примерно так:
// xcom-3dficator -fpck -w32 -h48 XCOM_0.PCK 32 33 34 35 36 37 38 39 > unarmored-male-head-torso.json

// В результате вываливаем список пикселов
// { points: [
//	{coord: {x: 15, y: 15, z: 17}, color: {r: 15, g: 36, b: 22}}
//	etc
// ]}

int main()
{
	// TODO: Нужно сделать нулевой verbosity level
	UP_RUN();

	cout << "Hello World!" << endl;
	return 0;
}

