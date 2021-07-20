#ifndef _WavRW_
#define _WavRW_

#include <vector>
#include <string>

using namespace std;

void audioWrite(const string &path, const vector<float> &x, int sr, int numCh);		//	Write an audio file from an interleaved buffer function intialization:

											//		path - path to audio file to write;
											//		x - audio data;
											//		sr - sampling rate;
											//		numCh - number of channels;

void audioWrite(const string &path, const vector<vector<float>> &x, int sr);		//	Write an audio file from an non-interleaved buffer function intialization:

											//		path - path to audio file to write;
											//		x - audio data;
											// 		sr - sampling rate;

void audioRead(const string &path, vector<float> &x, int& sr, int &numCh);		//	Read an audio file into non-interleaved buffer function intialization:

											//		path - path to audio file to write;
											//		x - audio data;
											//		sr - sampling rate;
											//		numCh - number of channels;

void audioRead(const string &path, vector<vector<float>> &x, int &sr);			//	Read an audio file into non-interleaved buffer function intialization:

											//		path - path to audio file to write;
											//		x - audio data;
											//		sr - sampling rate;
#endif
