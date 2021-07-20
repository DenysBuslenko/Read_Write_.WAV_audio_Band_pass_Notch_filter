#include "WavRW.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>

using namespace std;

typedef char FOURCC[4];		// Four character code - used to identify file format/data chunk;

static const int BITS_PER_SAMPLE = 16;

static const int MIN_SAMPLING_RATE = 8000;
static const int MAX_SAMPLING_RATE = 96000;

static void noninterleavedAudio(const vector<float>	&interleaved,		//	Interleaved file input;
				vector<vector<float>> 	&split,			//	Splitted file output;
				int			numCh		)	//	Number of channels;
{
	assert(numCh > 0 && numCh <= 2);

	float* srcPtr = (float*)interleaved.data();				//	srcPtr - storing an address of an each INTERLEAVED vector element;								
	int n = (int)interleaved.size() / numCh;				//	n - data used for one channel;									

	split.resize(numCh);							//	resizing SPLIT vector accordingly to the number of channels;

		for (int ch = 0; ch < numCh; ch++)				//	resizing SPLIT vector accordingly to N;								
	{
		split[ch].resize(n);
	}

	for (int i = 0; i < n; i++)						//	filling SPLIT vector;							
	{
		for (int ch = 0; ch < numCh; ch++)
		{
			split[ch][i] = *srcPtr++;
		}
	}
}

static void interleavedAudio(	vector<float>			&interleaved,		//	Interleaved file output;
				const vector<vector<float>>	&split		)	//	Split file input;		
{
	int numCh = (int)split.size();							//	numCh - determine number of channels;			

	assert(numCh > 0 && numCh <= 2);

	int n = (int)split[0].size();							//	n - data used for one channel;

	interleaved.resize(n*numCh);							//	resizing INTERLEAVED vector accrodingly to N and NumCH;

	float *outP = interleaved.data();						//	outP - storing an address of an each INTERLEAVED vector element;

	for (int i = 0; i < n; i++)							//	filling each channel of SPLIT vector with INTERLEAVED data;
	{
		for (int ch = 0; ch < numCh; ch++)
		{
			*outP++ = split[ch][i];
		}
	}
}

void checkProcessorEndianness()						//	Checking the endiannes of processor: 
{
	int x = 1;							//		Little-endian: [0x01|0x00|0x00|0x00] Least significant byte is first;
	bool littleEndian = (*(char*)(&x) == 1) ? true : false;		//		Big-endian:    [0x00|0x00|0x00|0x01] Least significant byte is last;

	assert(littleEndian);						//	http://stackoverflow.com/questions/12791864/c-program-to-check-little-vs-big-endian;
}

bool equalFourCC(const FOURCC a, const FOURCC b)			//	Returns true if FOURCC codes are equal;
{
	return (strncmp(a, b, 4) == 0);
}

int findChunk(istream &fp, const FOURCC	chunkIDToFind)			//	Searches from the current file position to find the chunk with the specified ID.

									//		fp - object to read and interpret input from sequences of characters;															
									//		chunkIDToFind - character sequence needed to find;										
{
	int count = 0;
	const int MAX_CHUNKS = 100;

	while (count < MAX_CHUNKS && !fp.eof())				//	While the end of the file haven't been reached;
	{
		long pos = fp.tellg();					//	Get the current position;

		count++;

		FOURCC chunkID;

		fp.read((char*)&chunkID, sizeof(chunkID));		//	Read chunk ID;

		if (fp.gcount() != sizeof(chunkID))
		{
			return 0;
		}

		int chunkSize;

		fp.read((char*)&chunkSize, sizeof(chunkSize));		//	Read chunk size;

		if (fp.gcount() != sizeof(chunkSize))
		{
			return 0;
		}

		if (equalFourCC(chunkID, chunkIDToFind))		//	If the chunk ID matches, return the chunk size;
		{
			return chunkSize;
		}

		fp.seekg(pos + chunkSize + 8);				//	Otherwise, skip this chunk and move to the next;
	}

	return 0;
}

int readWavHeader(	istream		&fp,				//	Reading a .WAV file header;
			int		&samplingRate,
			int		&numSamples,
			short		&numChannels,
			short		&bitsPerSample	)
{
	checkProcessorEndianness();					//	Checking the endiannes of processor;

	assert(sizeof(int) == 4);
	assert(sizeof(short) == 2);

	FOURCC chunkID;

	fp.read(chunkID, sizeof(chunkID));				//	The canonical WAVE format starts with the RIFF header:

	assert(fp.gcount() == sizeof(chunkID));				//	0         4   ChunkID		Contains the letters "RIFF" in ASCII form
	assert(equalFourCC(chunkID, "RIFF"));				//					(0x52494646 big-endian form);

	int chunkSize;							//	4         4   ChunkSize		36 + SubChunk2Size;

	fp.read((char*)&chunkSize, sizeof(chunkSize));

	assert(fp.gcount() == sizeof(chunkSize));

	fp.read(chunkID, sizeof(chunkID));				//	8         4   Format		Contains the letters "WAVE"
									//					(0x57415645 big-endian form);
	assert(fp.gcount() == sizeof(chunkID));
	assert(equalFourCC(chunkID, "WAVE"));

	long wavChunkPos = fp.tellg();

	// -- "fmt " subchunk --

	fp.seekg(wavChunkPos);

	int fmtChunkSize = findChunk(fp, "fmt ");

	assert(fmtChunkSize >= 16);

	short audioFormat;						//	20        2   AudioFormat	PCM = 1 (i.e. Linear quantization);

	fp.read((char*)&audioFormat, sizeof(audioFormat));

	assert(fp.gcount() == sizeof(audioFormat));
	assert(audioFormat == 1);

	fp.read((char*)&numChannels, sizeof(numChannels));		//	22        2   NumChannels	Mono = 1, Stereo = 2, etc;

	assert(fp.gcount() == sizeof(numChannels));
	assert(numChannels == 1 || numChannels == 2);

	fp.read((char*)&samplingRate, sizeof(samplingRate));		//	24        4   Sampling Rate	8000, 44100, etc;

	assert(fp.gcount() == sizeof(samplingRate));
	assert(samplingRate >= 8000 && samplingRate <= 96000);

	int byteRate;

	fp.read((char*)&byteRate, sizeof(byteRate));			//	28        4   ByteRate		== SamplingRate * NumChannels * BitsPerSample/8;

	assert(fp.gcount() == sizeof(byteRate));

	short blockAlign;						//	32        2   BlockAlign	== NumChannels * BitsPerSample/8;

	fp.read((char*)&blockAlign, sizeof(blockAlign));

	assert(fp.gcount() == sizeof(blockAlign));

	fp.read((char*)&bitsPerSample, sizeof(bitsPerSample));		//	34        2   BitsPerSample	8 bits = 8, 16 bits = 16, etc;

	assert(fp.gcount() == sizeof(bitsPerSample));
	assert(bitsPerSample % 8 == 0);
	assert(byteRate == samplingRate * numChannels * bitsPerSample / 8);
	assert(blockAlign == numChannels * bitsPerSample / 8);

	//	-- "data" subchunk --	\\

	fp.seekg(wavChunkPos);

	int dataChunkSize = findChunk(fp, "data");

	assert(dataChunkSize > 0);

	numSamples = dataChunkSize / blockAlign;

	return numSamples;
}

int writeWavHeader(	ostream		&fp,				//	Writing a .WAV file header;
			const int	samplingRate,
			const int	numSamples,
			const short	numChannels,
			const short	bitsPerSample	)
{
	checkProcessorEndianness();					//	Checking the endiannes of processor;

	assert(samplingRate >= 8000 && samplingRate <= 96000);
	assert(numSamples >= 0);
	assert(numChannels == 1 || numChannels == 2);
	assert(bitsPerSample == 8 || bitsPerSample == 16);
	assert(sizeof(int) == 4);
	assert(sizeof(short) == 2);
									//	The canonical WAVE format starts with the RIFF header:

	fp.write("RIFF", 4);						//	0         4   ChunkID		Contains the letters "RIFF" in ASCII form
									//					(0x52494646 big-endian form);

	int subChunk2Size = numSamples*numChannels*bitsPerSample / 8;
	int chunkSize = 36 + subChunk2Size;				//	4         4   ChunkSize		36 + SubChunk2Size;

	fp.write((char*)&chunkSize, sizeof(chunkSize));

	fp.write("WAVE", 4);						//	8         4   Format		Contains the letters "WAVE"
									//					(0x57415645 big-endian form);
	
									//	The "WAVE" format consists of two subchunks: "fmt " and "data";
									//	The "fmt" subchunk describes the sound data's format:

	fp.write("fmt ", 4);						//	12        4   Subchunk1ID	Contains the letters "fmt "
									//					(0x666d7420 big-endian form);

	int subChunk1Size = 16;

	fp.write((char*)&subChunk1Size, sizeof(subChunk1Size));		//	16        4   Subchunk1Size	16 for PCM. This is the size of the
									//					rest of the Subchunk which follows this number;
	short audioFormat = 1;

	fp.write((char*)&audioFormat, sizeof(audioFormat));		//	20        2   AudioFormat	PCM = 1;

	fp.write((char*)&numChannels, sizeof(numChannels));		//	22        2   NumChannels	Mono = 1, Stereo = 2, etc;

	fp.write((char*)&samplingRate, sizeof(samplingRate));		//	24        4   SamplingRate	8000, 44100, etc;

	int byteRate = samplingRate * numChannels * bitsPerSample / 8;

	fp.write((char*)&byteRate, sizeof(byteRate));			//	28        4   ByteRate		== SampleRate * NumChannels * BitsPerSample/8;

	short blockAlign = numChannels * bitsPerSample / 8;

	fp.write((char*)&blockAlign, sizeof(blockAlign));		//	32        2   BlockAlign	== NumChannels * BitsPerSample/8;
									//					The number of bytes for one sample including all channels;
	fp.write((char*)&bitsPerSample, sizeof(bitsPerSample));		//	34        2   BitsPerSample	8 bits = 8, 16 bits = 16, etc;

									//	The "data" subchunk contains the size of the data and the actual sound:

	fp.write("data", 4);						//	36        4   Subchunk2ID	Contains the letters "data"
									//					(0x64617461 big-endian form);

	fp.write((char*)&subChunk2Size, sizeof(subChunk2Size));		//	40        4   Subchunk2Size	== NumSamples * NumChannels * BitsPerSample/8;
									//					This is the number of bytes in the data;			
	return subChunk2Size;
}

void audioWrite(const string &path, const vector<float> &x, int sr, int numCh)			//	Write an audio file from an interleaved buffer;
{
	ofstream outStream(path.c_str(), ios::out | ios::binary);				//	Open the output .WAV file;

	assert(outStream.is_open());

	int numSamples = (int)x.size() / numCh;							//	Get the number of samples;

	assert(numSamples > 0);

	writeWavHeader(outStream, sr, numSamples, numCh, BITS_PER_SAMPLE);			//	Write the .WAV header;

	vector<short> shortBuf(numSamples*numCh);						//	Convert the samples from normalized [-1.0, 1.0] float to 16-bit shorts;

	float *src = (float*)x.data();
	short *dst = shortBuf.data();

	for (int i = 0; i < numSamples*numCh; i++)
	{
		float flt = *src++;

		flt = std::min<float>(flt, 1.0);
		flt = std::max<float>(flt, -1.0);

		*dst++ = SHRT_MAX * flt;
	}

	outStream.write((char*)shortBuf.data(), numCh*numSamples * sizeof(shortBuf[0]));	//	Write the audio data;

	outStream.close();
}

void audioWrite(const string& path, const vector<vector<float>> &x, int sr)			//	Write an audio file from an non-interleaved buffer;
{
	vector<float> interleaved;

	interleavedAudio(interleaved, x);

	audioWrite(path, interleaved, sr, (int)x.size());
}

void audioRead(const string& path, vector<float> &x, int &sr, int &numCh)			//	Read an audio file into an interleaved buffer;
{
	checkProcessorEndianness();								//	Checking the endiannes of processor;

	ifstream inStream(path.c_str(), ios::in | ios::binary);					//	Open an input .WAV file;

	if (!inStream.is_open())
	{
		printf("Couldn't open %s\n", path.c_str());

		assert(inStream.is_open());
	}

	int numSamples;
	short bitsPerSample;
	short numChannels;

	readWavHeader(inStream, sr, numSamples, numChannels, bitsPerSample);			//	Read the .WAV header;

	numCh = numChannels;

	assert(sr >= MIN_SAMPLING_RATE && sr <= MAX_SAMPLING_RATE);
	assert(numCh >= 1);
	assert(bitsPerSample % 8 == 0);

	int bytesPerSample = bitsPerSample / 8;

	x.resize(numCh*numSamples);								//	resizing an output vector X to accomodate the samples;

	vector<uint8_t> audioData(numChannels*numSamples*bytesPerSample);			//	Since the audio shall have been readed in multiple formats,
												//	it should be initialized as unsigned bytes;
	inStream.read((char*)audioData.data(), numCh*numSamples*bytesPerSample);

												// Convert to normalized [-1,1] floating point

	float maxSample = 1 << (bitsPerSample - 1);						//	Get absolute maximum sample value;

	float scale = 1.0 / maxSample;								//	Get scale factor to apply to normalize sum of
												//	samples across channels in a single sample frame;
	uint8_t *src = audioData.data();

	for (int i = 0; i < numCh*numSamples; i++)
	{
		int32_t sample = 0;
		uint8_t* sampleBytes = (uint8_t*)(&sample);					//	We assume this is little-endian;

		for (int b = 0; b < bytesPerSample; b++)					//	Read in the sample value byte-by-byte. Sample is assumed to be stored in
		{										//	little-endian format in the file, so least-significant byte is always first;
			sampleBytes[b] = *src++;
		}

		if (bytesPerSample == 1)							//	If it's 1 byte (8 bits) per sample;
		{
			sample += CHAR_MIN;							//	By convention, 8-bit audio is *unsigned* with sample values from 0 to 255.
		}										//	To make it signed, it's needed to be translated 0 to -128;

		else
			if (sampleBytes[bytesPerSample - 1] & 0x80)				//	If the most-significant bit of most-significant byte is 1, then the
			{									//	sample is negative. The upper bytes are needed to setted to all 1's;
				for (size_t b = bytesPerSample; b < sizeof(sample); b++)
				{
					sampleBytes[b] = 0xFF;
				}
			}

		x[i] = scale * sample;								//	Apply scale to get normalized mono sample value for this sample frame;
	}

	inStream.close();
}

void audioRead(const string& path, vector<vector<float>> &x, int &sr)				// Read into split (not interleaved) buffers
{
	int numCh;

	vector<float> interleaved;

	audioRead(path, interleaved, sr, numCh);

	noninterleavedAudio(interleaved, x, numCh);
}
