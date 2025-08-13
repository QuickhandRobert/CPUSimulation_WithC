#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>
#include "headers.h"
PaStream *audio_stream;
int bpm = DEFAULT_BPM;
int midiToFreq(unsigned char note) {
	return 440.0 * pow(2.0, (note - 69) / 12.0);
}
void squareWaveGenerate(int *buffer, int frames, int freq) {
	int period = AUDIO_SAMPLE_RATE / freq;
	for (int i = 0; i < frames; i++) {
		buffer[i] = (i % period < period / 2) ? AUDIO_AMPLITUDE : -AUDIO_AMPLITUDE;
	}
}
void audio_init() {
	Pa_Initialize();
	Pa_OpenDefaultStream(&audio_stream, 0, 1, paInt32, AUDIO_SAMPLE_RATE, paFramesPerBufferUnspecified, NULL, NULL);
}
void audio_shutdown() {
	Pa_CloseStream(audio_stream);
	Pa_Terminate();
}
void start_stream() {
	Pa_StartStream(audio_stream);
}
void stop_stream() {
	Pa_StopStream(audio_stream);
}
void setBPM (const int source_bpm) {
	bpm = source_bpm;
}
void play_note(unsigned char note, unsigned char duration) {
	int *buffer;
	int beat_duration = 60000 / bpm;
	int frames = AUDIO_SAMPLE_RATE * duration * beat_duration / 1000;
	buffer = (int *)malloc(frames * sizeof(int));
	if (note == 0) {
		memset(buffer, 0, frames * sizeof(int)); //Silence
	} else {
		int freq = midiToFreq(note);
		squareWaveGenerate(buffer, frames, freq);
	}
	Pa_WriteStream(audio_stream, buffer, frames);
	free(buffer);
}