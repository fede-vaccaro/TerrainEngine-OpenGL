#include "NoiseThread.h"

void NoiseThread::generateNoise(float*row, int row_index, int Wres, int Hres, float Hrange, float w, float h, int octaves, int primeIndex, double persistance){
 
	float HOffset = 0.0;

	int x = 0, offset = row_index * Wres;// *3;
	for (int i = 0; i < Wres; i ++) {
		//row[offset + i] = x/ (float)Wres * w - w / 2;
		row[offset + i + 1] = (float)ValueNoise_2D(x, row_index, octaves, primeIndex, persistance)* Hrange - HOffset;
		//row[offset + i + 2] = (float)row_index/(float)Hres * h - h / 2;
		//x++;
	}
	// Job done
}
NoiseThread::NoiseThread(float*row, int row_index, int Wres, int Hres, float Hrange,float w, float h, int octaves, int primeIndex, double persistance)
{
	t = this->spawn(row,row_index, Wres, Hres,Hrange, w, h, octaves, primeIndex, persistance);
}

std::thread* NoiseThread::spawn(float*row, int row_index, int Wres, int Hres, float Hrange,float w, float h, int octaves, int primeIndex, double persistance) {
	return new std::thread([this,row, row_index, Wres, Hres,Hrange, w, h, octaves, primeIndex, persistance] { this->generateNoise(row, row_index, Wres, Hres, Hrange, w, h, octaves, primeIndex, persistance); });
}

NoiseThread::~NoiseThread() {}

void NoiseThread::join()
{
	t->join();
}
