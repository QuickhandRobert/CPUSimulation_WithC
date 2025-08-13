#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define AMPLITUDE 3000

int midi_to_freq(uint8_t note) {
	return 440.0 * pow(2.0, (note - 69) / 12.0);
}

void generate_square_wave(int16_t *buffer, int frames, double freq) {
	int period = SAMPLE_RATE / freq;
	for (int i = 0; i < frames; i++) {
		buffer[i] = (i % period < period / 2) ? AMPLITUDE : -AMPLITUDE;
	}
}

int main(int argc, char *argv[]) {
	FILE *f = fopen("nokia.notes", "rb");
	if (!f) return 1;

	Pa_Initialize();
	PaStream *stream;
	Pa_OpenDefaultStream(&stream, 0, 1, paInt16, SAMPLE_RATE, paFramesPerBufferUnspecified, NULL, NULL);
	Pa_StartStream(stream);

	int bpm = 300;
	int ms_per_beat = 60000 / bpm;
	int16_t buffer[SAMPLE_RATE];

	while (!feof(f)) {
		int8_t note, duration;
		fread(&note, 1, 1, f);
		fread(&duration, 1, 1, f);

		int frames = SAMPLE_RATE * duration * ms_per_beat / 1000;
		if (note == 0) {
			memset(buffer, 0, frames * sizeof(int16_t));
		} else {
			int freq = midi_to_freq(note);
			generate_square_wave(buffer, frames, freq);


		}

		Pa_WriteStream(stream, buffer, frames);
	}
		Pa_StopStream(stream);
		Pa_CloseStream(stream);
		Pa_Terminate();
		fclose(f);
		return 0;
	}
