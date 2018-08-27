#pragma once
#include <iostream>
#include <cstdlib>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <ctime>
#include <windows.h>
#include <conio.h>

struct wav_header {
	char chunkId[4];
	unsigned int chunkSize;
	char format[4];
	char subchunk1Id[4];
	unsigned int subchunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	char subchunk2Id[4];
	unsigned int subchunk2Size;
};

struct wav {
	wav_header header;
	double* data;
};

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const double TwoPi_this = 6.283185307179586;

bool init();
void close();

wav* read_wav(char*fname);
void delete_wav(wav* smpl);
void FFTAnalysis(double *AVal, double *FTvl, int Nvl, int Nft);
int search_max(double* arr, int size);