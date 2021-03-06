#include "TransferFunction.h"

TransferFunction::TransferFunction() {
	transferFunction = (unsigned char*)malloc(256 * 4);
	preIntegrationTable = (unsigned char*)malloc(256 * 256 * 4);
}

TransferFunction::~TransferFunction() {
	delete [] transferFunction;
	delete [] preIntegrationTable;
}

void TransferFunction::load(char *path) {

	std::fstream file(path);
	std::string line;
		
	if(file.is_open()) {
	
		float opacity;
		int i, r, g, b, a;
		while(!file.eof()) {
			std::getline(file, line);
			i = atoi(line.c_str()); 
			std::getline(file, line);
			r = atoi(line.c_str());
			std::getline(file, line);
			g = atoi(line.c_str());
			std::getline(file, line);
			b = atoi(line.c_str());
			std::getline(file, line);
			a = atoi(line.c_str());
			opacity = (float)(a/255.0);
			transferFunction[i * 4 + 0] = r * opacity;
			transferFunction[i * 4 + 1] = g * opacity;
			transferFunction[i * 4 + 2] = b * opacity;
			transferFunction[i * 4 + 3] = a;
		}
		file.close();
	
	} else {
		std::cout << "Transfer Function could not be opened" << std::endl;
	}

}

int TransferFunction::clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

void TransferFunction::computePreIntegrationTable() {
	
	double r = 0, g = 0, b = 0, a = 0;
	int rcol, gcol, bcol, acol;
	double rInt[256], gInt[256], bInt[256], aInt[256];
	int smin, smax, preIntName;
	double factor, opacity;
	int lookupIndex = 0;
	rInt[0] = 0; gInt[0] = 0; bInt[0] = 0; aInt[0] = 0;
	//compute integral functions
	for(int i = 1; i < 256; i++) {
		opacity = ((transferFunction[(i-1) * 4 + 3] + transferFunction[i * 4 + 3])/2.0f);
		r = r + (transferFunction[(i-1) * 4 + 0] + transferFunction[i * 4 + 0])/2.0f * opacity/255.f;
		g = g + (transferFunction[(i-1) * 4 + 1] + transferFunction[i * 4 + 1])/2.0f * opacity/255.f;
		b = b + (transferFunction[(i-1) * 4 + 2] + transferFunction[i * 4 + 2])/2.0f * opacity/255.f;
		a = a + opacity;
		rInt[i] = r; gInt[i] = g; bInt[i] = b; aInt[i] = a;
	}
	//compute look-up table from integral functions
	for(int sb = 0; sb < 256; sb++) {
		for(int sf = 0; sf < 256; sf++) {
			
			if(sb < sf) {
				smin = sb;
				smax = sf;
			} else {
				smin = sf;
				smax = sb;
			}

			if(smax != smin) {
				factor = 1.0f / (double)(smax - smin);
				rcol = (rInt[smax] - rInt[smin]) * factor;
				gcol = (gInt[smax] - gInt[smin]) * factor;
				bcol = (bInt[smax] - bInt[smin]) * factor;
				acol = 256.0f * (1.0f - exp(-(aInt[smax] - aInt[smin]) * factor/255.f));
			} else {
				factor = 1.0f / 255.f;	
				rcol = transferFunction[smin * 4 + 0] * transferFunction[smin * 4 + 3] * factor;
				gcol = transferFunction[smin * 4 + 1] * transferFunction[smin * 4 + 3] * factor;
				bcol = transferFunction[smin * 4 + 2] * transferFunction[smin * 4 + 3] * factor;
				acol = (1.0f - exp(-transferFunction[smin * 4 + 3] * 1.0f / 255.f))*256.f;
			}

			preIntegrationTable[(sb * 256 + sf) * 4 + 0] = clamp(rcol * 1.5, 0, 255);
			preIntegrationTable[(sb * 256 + sf) * 4 + 1] = clamp(gcol * 1.5, 0, 255);
			preIntegrationTable[(sb * 256 + sf) * 4 + 2] = clamp(bcol * 1.5, 0, 255);
			preIntegrationTable[(sb * 256 + sf) * 4 + 3] = clamp(acol, 0, 255);

		}
	}

}