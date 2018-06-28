#pragma once
#include <thread>
#include <iostream>
#include "PerlinNoise.h"

class NoiseThread
{
private:
	int* Semaphore;
	std::thread* t;
	void generateNoise(float*row, int row_index, int Wres, int Hres, float Hrange, float w, float h, int octaves, int primeIndex, double persistance);
	std::thread* spawn(float*row, int row_index, int Wres, int Hres, float Hrange, float w, float h, int octaves, int primeIndex, double persistance);

public:
	NoiseThread(float*row, int row_index, int Wres, int Hres, float Hrange, float w, float h, int octaves, int primeIndex, double persistance);
	~NoiseThread();
	void join();
};

