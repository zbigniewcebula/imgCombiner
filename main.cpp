#if 0
#!/bin/bash
g++ $0 lodepng.cpp -g --std=c++11 -lpng -lz -lX11 -lpthread -o main.exe
mv main.exe ./build/
exit
#endif

#define MAJOR	0
#define MINOR	1

#include <iostream>
#include <map>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>
#ifdef _WIN32
	#include <windows.h>
#endif

#include <dirent.h>

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;

#include "lodepng.h"

using namespace std;

uint	outWidth		= 32;
uint	outHeight		= 32;
string	outFile			= "out.png";
string	outUniqueFile	= "";

bool fileExists(string name);
bool dirExists(string name);

bool isInt(string str);

vector<string> listFiles(string dirName, string ext = "");

void showHelp();
string version();

int main(int argc, char** argv) {
	//Preparing command line arguments
	vector<string>	args;
	if (argc > 1) {
		for(int i = 1; i < argc; ++i) {
			args.push_back(string(argv[i]));
		}
	} else {
		args.push_back("--help");
	}
	//////////////////////////////
	vector<string>			layers;
	vector<vector<string>>	layerFiles;
	vector<int>		usedRandoms;

	srand(time(0));

	//////////////////////////////
	string	badArg	= "";
	int		cArg	= 0;
	string	lastArg;
	bool	silentGetOut	= false;
	for_each(args.begin(), args.end(), [&](string arg)->void {
		if (lastArg == "-w" || lastArg == "-h") {
			if (isInt(arg)) {
				if (lastArg == "-w") {
					outWidth	= atoi(arg.c_str());
				} else if (lastArg == "-h") {
					outHeight	= atoi(arg.c_str());
				}
				if (outWidth == 0 || outHeight == 0) {
					badArg		= "Width/Height cannot be 0";
				}
			} else {
				badArg		= "Width/Height incorrect value";
			}
		} else if (lastArg == "-f") {
			if (fileExists(arg)) {
				badArg		= "Out file exists or path is inaccesible";
			} else {
				outFile		= arg;
			}
		} else if (lastArg == "-s") {
			if (isInt(arg)) {
				srand(atoi(arg.c_str()));
			} else {
				badArg		= "Seed must be a integer value";
			}
		} else if (lastArg == "-u") {
			if (!dirExists(arg)) {
				badArg			= "Output directory not exists or path is inaccesible: " + arg;
			} else {
				outUniqueFile	= arg;
			}
		} else if (
			arg == "-w"
		||	arg == "-h"
		||	arg == "-s"
		||	arg == "-f"
		||	arg == "-u"
		) {
			lastArg	= arg;
		} else if (arg == "--help") {
			showHelp();
			silentGetOut	= true;
		} else if (arg == "-v") {
			cout	<<	version() << '\n';
			silentGetOut	= true;
		} else {
			if (!dirExists(string("./") + arg)) {
				badArg		= string("Input directory not exists or path is inaccesible: ") + string("./") + arg;
			} else {
				layers.push_back(arg);
			}
		}
		lastArg	= arg;
		++cArg;
	});
	if (silentGetOut) {
		return 0;
	}
	if (layers.size() == 0) {
		badArg	= "No layers directories given";
	} else {
		if (any_of(layers.begin(), layers.end(), [](string dir)->bool {
			return !dirExists(dir);
		})) {
			badArg	= "One of the layers directory does not exists";
		} else {
			for(int i = 0; i < layers.size(); ++i) {
				layerFiles.emplace_back(listFiles(layers[i], ".png"));
			}
		}
	}
	if (badArg.length() > 0) {
		cerr	<< "[imgCombiner] Incorrect argument(s): " << badArg << endl;
		return 1;
	}

	//Creating image
	vector<uchar>	out(outWidth * outHeight * 4, 0);

	for(int i = 0; i < layers.size(); ++i) {
		//Select random file from layer 
		usedRandoms.push_back(rand()%layerFiles[i].size());
		int	randomFileIndex	= usedRandoms.back();

		//Load random file
		unsigned		width, height;
		vector<uchar>	in;
		unsigned		error	= lodepng::decode(
			in, width, height,
			((layers[i] + "/") +  layerFiles[i][randomFileIndex]).c_str()
		);
		if (error) {
			cerr	<<  "[LODEPNG] decoder error " << error << ": " << lodepng_error_text(error) << endl;
			cerr	<<	"[imgCombiner] Error at file: " << layers[i] + "/" +  layerFiles[i][usedRandoms.back()] << endl;
			return 1;
		} else if (height > outHeight || width > outWidth) {
			cerr	<<	"[imgCombiner] Layer image cannot be bigger than output image. Aborting" << endl;
			return 1;
		}

		//Copy pixels from input layer into output file
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				if (in[y * width * 4 + x * 4 + 3] > 0) {
					out[y * width * 4 + x * 4 + 0]	= in[y * width * 4 + x * 4 + 0];
					out[y * width * 4 + x * 4 + 1]	= in[y * width * 4 + x * 4 + 1];
					out[y * width * 4 + x * 4 + 2]	= in[y * width * 4 + x * 4 + 2];
					out[y * width * 4 + x * 4 + 3]	= 0xFF;
				}
			}
		}
	}

	//Saving
	vector<unsigned char>	png;
	unsigned				error	= lodepng::encode(png, out, outWidth, outHeight);
	if (outUniqueFile.length() > 0) {
		if(!error) {
			stringstream	ss;
			for_each(usedRandoms.begin(), usedRandoms.end(), [&](int random)->void {
				ss	<< random << "_";
			});

			lodepng::save_file(png, outUniqueFile + ss.str() + ".png");
		}
	} else if(!error) {
		lodepng::save_file(png, outFile);
	} else {
		cerr	<< "[LODEPNG] Encoder error " << error << ": "<< lodepng_error_text(error) << endl;
	}
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

bool isInt(string str) {
	return all_of(str.begin(), str.end(), [](char c)->bool {
		return isdigit(c);
	});
}

vector<string> listFiles(string dirName, string ext) {
	DIR*			dir		= opendir(dirName.c_str());
	dirent*			ent		= NULL;
	vector<string>	list;

	if (dir != NULL) {
		while((ent = readdir(dir)) != NULL) {
			string	fileName(ent->d_name);
			if (fileName == "." || fileName == "..") {
				continue;
			}
			//If no ext => add anyway
			//If ext => add only when *fileName* ends with *ext*
			if (ext.length() == 0
			||	fileName.substr(fileName.length() - ext.length()) == ext
			) {
				list.push_back(fileName);
			}
			
		}
		closedir(dir);
	}
	return list;
}

void showHelp() {
	cout	<<	"imgCombiner [FLAG [VALUE]] layer_1 layer_2 layer_n...\n"
			<<	"  Merges random images (on top) from given directories (layers) in given order\n"
			<<	'\n'
			<<	"	FLAG        VALUE       DESCRIPTION\n"
			<<	"	--help                  shows help for available flags\n"
			<<	"	-h          number      sets width for output image\n"
			<<	"	-w          number      sets height for output image\n"
			<<	"	-f          name        sets file name for output image (PNG FORMAT!)\n"
			<<	"	-u          dir         sets output directory for image iwth unique name\n"
			<<	"	-s          number      sets seed for random number generator\n"
			<<	"	-v                      shows only name and version\n"
			<<	"\n"
			<<	"version:	" << version() << '\n'
	<< endl;
}

string version() {
	stringstream	ss;
	ss << MAJOR << "." << MINOR;
	return ss.str();
}