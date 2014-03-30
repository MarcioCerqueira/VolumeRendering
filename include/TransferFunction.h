#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <malloc.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>

class TransferFunction
{
public:
	TransferFunction();
	~TransferFunction();
	void load(char *path);
	int clamp(int x, int a, int b);
	void computePreIntegrationTable();

	unsigned char* getTransferFunction() { return transferFunction; }
	unsigned char* getPreIntegrationTable() { return preIntegrationTable; }
private:
	unsigned char *transferFunction;
	unsigned char *preIntegrationTable;
};
#endif