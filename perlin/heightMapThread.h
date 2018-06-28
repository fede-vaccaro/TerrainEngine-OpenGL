#pragma once
#include <thread>
#include <iostream>
#include "PerlinNoise.h"

typedef struct{
	float max, min;
}threadValues;

class heightMapThread
{
private:
	threadValues return_values;
	std::thread* t;
	void generateNoise(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance);
	std::thread* spawn(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance);

public:
	heightMapThread(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance);
	~heightMapThread();
	threadValues join();
};