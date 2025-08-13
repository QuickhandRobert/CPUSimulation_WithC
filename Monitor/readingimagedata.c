#include <stdio.h>
#include <ctype.h>
#include <windows.h>
int main() {
	int *image_data;
	int pixel;
	int r, g, b;
	FILE *f = fopen("minimap.raw", "rb");
	int image_bounds[] = {3072, 3072};
//	fread((void *)&image_bounds, sizeof(int), 2, f);
	image_data = (int *)malloc(sizeof(int) * image_bounds[0] * image_bounds[1]);
	for (int i = 0; i < image_bounds[0] * image_bounds[1]; i++) {
		r = (unsigned char)fgetc(f);
		g = (unsigned char)fgetc(f);
		b = (unsigned char)fgetc(f);
		image_data[i] = (r << 16) | (g << 8) | b;
	}
	for (int i = 0; i < image_bounds[0] * image_bounds[1]; i++) {
		char buffer[256];
		pixel = image_data[i];
		r   = (pixel >> 16) & 0xFF;
		g = (pixel >> 8)  & 0xFF;
		b  =  pixel        & 0xFF;
		printf("\x1b[48;2;%d;%d;%dm \x1b[0m", r, g, b); // space with background
		if(((i + 1) % image_bounds[0]) == 0)
			 putchar('\n');
	}
	free(image_data);
}