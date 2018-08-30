#include "Header.h"

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;

SDL_Surface* gSurface = NULL;

int main(int argc, char* argv[]) {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	if (!init())
		printf("Failed to initializate!\n");

	wav* smpl = read_wav("440.wav");
	wav* smpl2 = read_wav("1k.wav");
	int num = 1024;
	double* data_out = new double[num];
	double* data_disp = smpl->data + smpl->header.subchunk2Size / 2000;
	FFTAnalysis(data_disp, data_out, num, num);
	//for (int i = 0; i < num; i++)
	//	printf("%lf\n", data_out[i]);
	//printf("index=%d, value=%lf\n", search_max(data_out, num));
	
	int midx = SCREEN_WIDTH / 2, midy = SCREEN_HEIGHT / 2;
	int k = 40;
	int up = 0;

	int flag = 0;

	bool quit = false;
	SDL_Event e;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case '-':
					k--;
					break;
				case '=':
					k++;
					break;
				case SDLK_UP:
					up-=10;
					break;
				case SDLK_DOWN:
					up+=10;
					break;
				}
			}
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		for (int i = 0; i < smpl->header.subchunk2Size/2/20; i++) {
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
			float x = i * 1000.0 / smpl->header.sampleRate, y = smpl->data[i];
			SDL_RenderDrawPoint(gRenderer, x*k, midy - y*k);

			SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
			float x2 = i * 1000.0 / smpl2->header.sampleRate, y2 = smpl2->data[i];
			SDL_RenderDrawPoint(gRenderer, x2*k, midy - y2*k);

			SDL_SetRenderDrawColor(gRenderer, 0xAA, 0x99, 0x99, 0xFF);
			float x3 = i * 1000.0 / smpl2->header.sampleRate, y3 = 0.68*sin(440*6.2830*i/smpl->header.sampleRate);
			SDL_RenderDrawPoint(gRenderer, x3*k, midy + up - y3*k);

			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
			float x4 = i * smpl->header.sampleRate/num/10, y4 = data_out[i];
			SDL_RenderDrawPoint(gRenderer, x4, midy + midy * 3 / 4 - y4*k);
		}
		SDL_RenderPresent(gRenderer);
		//SDL_RenderPresent(gRenderer);
	}
	
	delete_wav(smpl);
	delete_wav(smpl2);
	delete[]data_out;
	close();
	return 0;
}

bool init() {
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		success = false;
	}
	else {
		gWindow = SDL_CreateWindow("DefaultWindow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL) {
				printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					printf("SDL_image Error: %s\n", SDL_GetError());
					success = false;
				}
			}
		}
	}
	return success;
}

void close() {
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(gRenderer);

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	IMG_Quit();
	SDL_Quit();
}

wav* read_wav(char*fname) {
	FILE *f;
	errno_t err;
	if (err = fopen_s(&f, fname, "rb")) {
		printf("Failed to open file! Error %d\n", err);
		return NULL;
	}

	wav* wav1 = new wav;

	fread_s(&wav1->header, sizeof(wav_header), sizeof(wav_header), 1, f);
	long size = wav1->header.subchunk2Size;
	if(wav1->header.numChannels == 2) return NULL;
	//TODO: stereo reading ---^
	char* buffer1 = new char[size];
	char* buffer2 = buffer1;
	double* data = new double[size / 2];
	fread_s(buffer1, size, sizeof(char), size, f);

	for (int i = 0; i < size; i += 2) {
		double temp1 = (double)*buffer2;
		buffer2++;
		double temp2 = (double)*buffer2;
		buffer2++;

		temp1 = (temp2 * 255.0 + temp1) / 32767;
		*(data + i / 2) = temp1;
	}
	wav1->data = data;

	fclose(f);

	delete[]buffer1;
	return wav1;
}

void delete_wav(wav* smpl) {
	delete[]smpl->data;
	delete smpl;
}

void FFTAnalysis(double *AVal, double *FTvl, int Nvl, int Nft) {
	int i, j, n, m, Mmax, Istp;
	double Tmpr, Tmpi, Wtmp, Theta;
	double Wpr, Wpi, Wr, Wi;
	double *Tmvl;

	n = Nvl * 2; Tmvl = new double[n];

	for (i = 0; i < n; i += 2) {
		Tmvl[i] = 0;
		Tmvl[i + 1] = AVal[i / 2];
	}

	i = 1; j = 1;
	while (i < n) {
		if (j > i) {
			Tmpr = Tmvl[i]; Tmvl[i] = Tmvl[j]; Tmvl[j] = Tmpr;
			Tmpr = Tmvl[i + 1]; Tmvl[i + 1] = Tmvl[j + 1]; Tmvl[j + 1] = Tmpr;
		}
		i = i + 2; m = Nvl;
		while ((m >= 2) && (j > m)) {
			j = j - m; m = m >> 1;
		}
		j = j + m;
	}

	Mmax = 2;
	while (n > Mmax) {
		Theta = -TwoPi_this / Mmax; Wpi = sin(Theta);
		Wtmp = sin(Theta / 2); Wpr = Wtmp * Wtmp * 2;
		Istp = Mmax * 2; Wr = 1; Wi = 0; m = 1;

		while (m < Mmax) {
			i = m; m = m + 2; Tmpr = Wr; Tmpi = Wi;
			Wr = Wr - Tmpr * Wpr - Tmpi * Wpi;
			Wi = Wi + Tmpr * Wpi - Tmpi * Wpr;

			while (i < n) {
				j = i + Mmax;
				Tmpr = Wr * Tmvl[j] - Wi * Tmvl[j - 1];
				Tmpi = Wi * Tmvl[j] + Wr * Tmvl[j - 1];

				Tmvl[j] = Tmvl[i] - Tmpr; Tmvl[j - 1] = Tmvl[i - 1] - Tmpi;
				Tmvl[i] = Tmvl[i] + Tmpr; Tmvl[i - 1] = Tmvl[i - 1] + Tmpi;
				i = i + Istp;
			}
		}

		Mmax = Istp;
	}

	for (i = 0; i < Nft; i++) {
		j = i * 2; FTvl[i] = 2 * sqrt(pow(Tmvl[j], 2) + pow(Tmvl[j + 1], 2)) / Nvl;
	}

	delete[]Tmvl;
}

int search_max(double* arr, int size) {
	double max = arr[0];
	int index_max = 0;
	for(int i = 0; i < size; i++)
		if (arr[i] > max) {
			max = arr[i];
			index_max = i;
		}
	return index_max;
}