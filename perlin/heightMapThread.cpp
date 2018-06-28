#include "heightMapThread.h"

void heightMapThread::generateNoise(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance) {

	float HOffset = 0.0;

	int x = 0, offset = row_index * Wres;
	float value;
	return_values.max = 0;
	return_values.min = 0;
	for (int i = 0; i < Wres; i ++) {
		value = (float)ValueNoise_2D(x++, row_index, octaves, primeIndex, persistance);
		row[offset + i + 1] = value;
		if (return_values.min > value)
			return_values.min = value;
		if (return_values.max < value)
			return_values.max = value;
	}
	// Job done
}
heightMapThread::heightMapThread(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance)
{
	t = this->spawn(row, row_index, Wres, Hrange, octaves, primeIndex, persistance);
}

std::thread* heightMapThread::spawn(float*row, int row_index, int Wres, float Hrange, int octaves, int primeIndex, double persistance) {
	return new std::thread([this, row, row_index, Wres, Hrange,octaves, primeIndex, persistance] { this->generateNoise(row, row_index, Wres, Hrange, octaves, primeIndex, persistance); });
}

heightMapThread::~heightMapThread() {}

threadValues heightMapThread::join()
{
	t->join();
	return return_values;
}