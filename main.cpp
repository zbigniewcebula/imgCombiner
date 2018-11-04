#if 0
#!/bin/bash
g++ $0 --std=c++11 -L/usr/X11R6/lib -lm -lpthread -lX11 -o main.exe
mv main.exe ./build/
exit
#endif

#pragma comment(lib, "gdi32.lib")
//cls && g++ main.cpp --std=c++11 -lgdi32 -o main.exe && main.exe

#include <iostream>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#ifdef _WIN32
	#include <windows.h>
#endif

#include <dirent.h>

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;

#include "CImg.h"

using namespace std;
using namespace cimg_library;

uint	outWidth	= 32;
uint	outHeight	= 32;
string	outFile		= "out.png";

bool fileExists(string name);
bool dirExists(string name);

int main(int argc, char** argv) {
	vector<string>	args;
	if (argc > 1) {
		for(int i = 1; i < argc; ++i) {
			args.push_back(string(argv[i]));
		}
	} else {
		cerr	<< "No arguments! Aborting." << endl;
		return 1;
	}
	string	badArgs	= "";
	int		cArg	= 0;
	string	lastArg;
	for_each(args.begin(), args.end(), [&](string arg)->void {
		if (lastArg == "-w" || lastArg == "-h") {
			if (all_of(arg.begin(), arg.end(), [&](char c)->bool {
				return bool(isdigit(c));
			})) {
				if (lastArg == "-w") {
					outWidth	= atoi(arg.c_str());
				} else if (lastArg == "-h") {
					outHeight	= atoi(arg.c_str());
				}
				if (outWidth == 0 || outHeight == 0) {
					badArgs		= "Width/Height cannot be 0";
				}
			} else {
				badArgs		= "Width/Height incorrect value";
			}
		} else if (lastArg == "-f") {
			if (fileExists(arg)) {
				badArgs		= "Out file exists or path is inaccesible";
			} else {
				outFile		= arg;
			}
		}
		lastArg	= arg;
		++cArg;
	});
	if (badArgs.length() > 0) {
		cerr	<< "Incorrect argument: " << badArgs << endl;
		return 1;
	}

	CImg<uchar> image(outWidth, outHeight, 1, 4);

	image.save(outFile.c_str());
	return 0;
}

bool fileExists(string name) {
	FILE*	h	= fopen(name.c_str(), "rw");
	if (h == NULL) {
		return false;
	}
	fclose(h);
	return true;
}
bool dirExists(string name) {
	DIR*	dir	= opendir(name.c_str());
	if (dir) {
		closedir(dir);
		return true;
	}
	return false;
}