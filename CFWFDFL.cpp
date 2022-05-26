/*
* =============================================================================
*
* This program calculates the epsilonX, epsilonY, Poission Ratio and Width
* of a GO system from the LAMMPS cfg files
*
* Author:  Mahdi Tavakol (mahditavakol90@gmail.com)
* Version: 09.25.2016
*
*
* =============================================================================
*/


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <random>
#include "Atoms.h"
#include "Atoms.cpp"
using namespace std;

int main()
{
	int frameNumber;
	int atomCount;
	float boxLengthX, boxLengthY, boxLengthZ;
	stringstream frameNumberStream;
	string dumpFileName;
	string outputFileName = "width.txt";
	ofstream outputFile(outputFileName, ios::out);
	string frameNumberString;
	string line;
	int    endFlag = 0;
	float  previousEpsilonX = 0.0f;
	float  previousEpsilonY = 0.0f;
	float  intialWidth = 0.0f;
	cout << "Welcome to LAMMPS dump reader written by" << endl;
	cout << "Somebody else" << endl << endl << endl;
	// outputFile << "LAMMPS Dump Reader written by" << endl;
	// outputFile << "Somebody else" << endl;
	outputFile << "epsilonX" << "---";
	outputFile << "---" << "epsilonY";
	outputFile << "---" << "Poisson Ratio";
	outputFile << "---" << "Width";
	outputFile << endl;
	// int frames[] = { 24000,3024000,3336000, 4000000, 5000000,600000, 700000,8000,9,10,11,12 }; //Debugger
	for (int i = 0; i < 500; i++)
	{
		// frameNumber = frames[i]; //Debugger
		frameNumber = (i + 1) * 24000;
		frameNumberStream.str(std::string()); //cleasing the streamstring
		frameNumberStream << frameNumber;
		frameNumberString = frameNumberStream.str();
		dumpFileName = "dump." + frameNumberString + ".cfg";

		ifstream dumpFile(dumpFileName, ios::in);

		if (dumpFile.is_open())
		{
			std::getline(dumpFile, line); // 1st line
			line.erase(line.find("Number of particles ="), 21);
			stringstream(line) >> atomCount;
			std::getline(dumpFile, line); // 2nd line (skipped)
			std::getline(dumpFile, line); // 3rd line
			line.erase(line.find("H0(1,1) ="), 9);
			line.erase(line.find("A"), 1);
			stringstream(line) >> boxLengthX;
			std::getline(dumpFile, line); // 4st line
			std::getline(dumpFile, line); // 5st line
			std::getline(dumpFile, line); // 6st line
			std::getline(dumpFile, line); // 7st line
			line.erase(line.find("H0(2,2) ="), 9);
			line.erase(line.find("A"), 1);
			stringstream(line) >> boxLengthY;
			std::getline(dumpFile, line); // 8st line
			std::getline(dumpFile, line); // 9st line
			std::getline(dumpFile, line); // 10st line
			std::getline(dumpFile, line); // 11st line
			line.erase(line.find("H0(3,3) ="), 9);
			line.erase(line.find("A"), 1);
			stringstream(line) >> boxLengthZ;
			std::getline(dumpFile, line); // 12st line
			std::getline(dumpFile, line); // 13st line
		}
		else
		{
			cout << "There is no " << dumpFileName << " file" << endl << endl;
			endFlag++;
			if (endFlag > 5)
			{
				cout << "All Done, Back To You OS" << endl << endl << endl;
				system("PAUSE");
				return 0;
			}
			continue;
		}


		Atom* atom = new Atom[atomCount];


		cout << "Reading Input Frame Number " << frameNumber << endl;
		for (int j = 0; j < atomCount; j++)
		{
			std::getline(dumpFile, line);
			int atomMass;
			stringstream(line) >> atomMass;
			std::getline(dumpFile, line);
			char atomChar;
			stringstream(line) >> atomChar;
			std::getline(dumpFile, line);
			float x, y, z;
			stringstream(line) >> x >> y >> z;
			int atomID = j;

			// output << line << endl; Since we might want to change the type of some carbon atoms
			// to be incorporated in the united atom model, we prefer to write atoms again.
			atom[j].SetAtom(atomID, atomMass, x, y, z, true, atomChar);
		}

		cout << "Reading The Input File Is Completed" << endl;
		cout << "Calculating the width of the middle strip " << endl << endl << endl << endl;

		dumpFile.close();
		dumpFile.clear();

		// float ymax = 0.0f;
		// float ymin = 1.0f;
		float xmiddle = 0.5f;
		float stripLength = 0.01f;
		float BCLength = 0.2f;
		float stripMin = 0.0f + BCLength / 2.0f;
		float stripMax = 1.0f - BCLength / 2.0f;
		int numberOfStrips = (int)((1.0f - BCLength) / stripLength);
		float* ymax = new float[ numberOfStrips ];
		float* ymin = new float[ numberOfStrips ];
		float* xmax = new float[ numberOfStrips ];
		float* xmin = new float[ numberOfStrips ];
		float width = 0;

		for (int j = 0; j < numberOfStrips; j++)
		{
			ymax[j] = 0.0f;
			ymin[j] = 1.0f;
			xmax[j] = stripMin + ((float)(j + 1)) * stripLength;
			xmin[j] = stripMin + ((float)j) * stripLength;
		}

		for (int j = 0; j < atomCount; j++)
		{
			float x, y, z;
			atom[j].GetCoordinates(x, y, z);
			bool flag = false;

			if (flag) continue;

			for (int k = 0; k < numberOfStrips; k++)
			{
				if (x < xmax[k] && x > xmin[k])
				{
					flag = true;
					if (y > ymax[k])
						ymax[k] = y;
					else if (y < ymin[k])
						ymin[k] = y;
				}
			}
		}

		for (int j = 0; j < numberOfStrips; j++)
		{
			width += ymax[j] - ymin[j];
		}

		width /= numberOfStrips;
		width *= boxLengthY;

		if (i)
		{
			float epsilonX = ((float) ((frameNumber - 24000) / 24000)) * 0.00225f;
			float epsilonY = (width - intialWidth) / intialWidth;
			float poissonRatio = -(epsilonY - previousEpsilonY) / (epsilonX - previousEpsilonX);
			outputFile << epsilonX << " " ;
			outputFile << " " << epsilonY;
			outputFile << " " << poissonRatio;
			outputFile << " " << width;
			outputFile << endl;
			previousEpsilonX = epsilonX;
			previousEpsilonY = epsilonY;
		}
		else
		{
			intialWidth = width;
			float epsilonX = 0;
			float epsilonY = 0;
			outputFile << epsilonX << " ";
			outputFile << " " << epsilonY;
			outputFile << " " << "----------";
			outputFile << " " << width;
			outputFile << endl;
		}
		delete[] atom;

	}



	cout << "All Done, Back To You OS" << endl << endl << endl;

	system("PAUSE");
	return 0;
	//*********************************************************************//
	//*********************************************************************//
	//*********************************************************************//
	//*********************************************************************//
	//*********************************************************************//
}
