#include <cstdio>
#include <cmath>
#include <ctgmath>
#include <vector>
#include <string>
#include <cassert>

#include "WavRW.h"
#include "Biquad.h"

using namespace std;

const int SAMPLING_RATE = 44100;

const string IN_DIR = "C:\\Users\\Buslenko\\Desktop\\Test_task_Buslenko\\Test_Task_Buslenko\\Audio_IN\\";
const string OUT_DIR = "C:\\Users\\Buslenko\\Desktop\\Test_task_Buslenko\\Test_Task_Buslenko\\Audio_OUT\\";


void readWriteAudio()								//	Read a mono audio file, write to an output file.
{
	printf("ReadWriteAudio\n");

	vector<float> sourceBuf;						//	Read the input file

	int sr;
	int numCh;

	string sourcePath = IN_DIR + "Stairway_Mono.wav";

	audioRead(sourcePath, sourceBuf, sr, numCh);

	auto numFrames = sourceBuf.size();

	vector<float> outBuf(numFrames);					//	Set up an output buffer

	for (int i = 0; i < numFrames; i++)					//	For each output frame
	{
		float x = sourceBuf[i];						//	Get input sample

		float gain = 0.5;						//	Copy it to the output buffer

		outBuf[i] = gain * x;
	}

	string outPath = OUT_DIR + "Stairway_Mono_1.wav";			//	Write the audio to file

	audioWrite(outPath, outBuf, SAMPLING_RATE, 2);
}

void applyFilter()								//	Apply a filter
{
	printf("applyFilter\n");

	vector<float> sourceBuf;						//	Read the input file

	int sr;
	int numCh;

	string sourcePath = IN_DIR + "Stairway_Mono.wav";

	audioRead(sourcePath, sourceBuf, sr, numCh);

	auto sourceLen = sourceBuf.size();

	vector<float> outBuf(sourceLen);					//	Set up an output buffer

	Biquad filter(SAMPLING_RATE);						//	Set up a filter

	float cutoffFreqHz = 400;
	filter.initBPF(cutoffFreqHz);

	for (int i = 0; i < sourceLen; i++)					//	For each output frame			
	{
		float x = sourceBuf[i];						//	Get the input sample

		float y = filter.tick(x);					//	Pass it to the filer

		outBuf[i] = y;							//	And put it in the output buffer
	}

	string outPath = OUT_DIR + "Stairway_Mono_Filter_Out.wav";		//	Write the audio to file

	audioWrite(outPath, outBuf, SAMPLING_RATE, 2);
}

int main()
{
	printf("Test task\n");

	readWriteAudio();
	applyFilter();

	printf("Done ;)\n");

	return 0;
}
