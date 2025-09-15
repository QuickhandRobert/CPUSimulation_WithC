#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#define BUFFER_SIZE 16384
#define FILENAME_MAXLEN 256
int main(int argc, char *argv[]){ //drive_maker OS_filename output_filename drive_size cluster_size
	FILE *fp = fopen(argv[2], "wb");
	FILE *curr_file;
	fprintf(fp, "%s %s\n", argv[3], argv[4]);
	uint8_t *file_buffer = malloc(BUFFER_SIZE * sizeof(uint8_t));
	DIR *cur_dir;
	struct dirent *file_data;
	cur_dir = opendir("./");
	curr_file = fopen(argv[1], "rb");
	fpos_t lastpos;
	fprintf(fp, "%s 1 1\n", argv[1]);
	fgetpos(fp, &lastpos);
	size_t read_data = fread(file_buffer, sizeof(uint8_t), BUFFER_SIZE, curr_file);
	fseek(fp, BUFFER_SIZE, SEEK_SET);
	fwrite(file_buffer, sizeof(uint8_t), read_data, fp);
	fsetpos(fp, &lastpos);
	for (int i = 2; (file_data = readdir(cur_dir)) != NULL; i++){
		if (*file_data->d_name == '.' || strcmp(file_data->d_name, argv[1]) == 0 || strcmp(file_data->d_name, argv[0]) == 0 || strcmp(file_data->d_name, argv[2]) == 0){
			i--;
			continue;
		}
		curr_file = fopen(file_data->d_name, "rb");
		fprintf(fp, "%s 1 %d\n", file_data->d_name, i);
		fgetpos(fp, &lastpos);
		read_data = fread(file_buffer, sizeof(uint8_t), BUFFER_SIZE, curr_file);
		fseek(fp, i * BUFFER_SIZE, SEEK_SET);
		fwrite(file_buffer, sizeof(uint8_t), read_data, fp);
		fsetpos(fp, &lastpos);
	}
	fprintf(fp, "END\n");
	free(file_buffer);
}


