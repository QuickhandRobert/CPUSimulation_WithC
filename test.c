#include <stdio.h>
int getword(outputWord[10]){
	int c, i;
	for (i = 0; c = getchar() != EOF; i++){
		outputWord[i] = c;
	}
	outputWord[i] = '\0';
}
int main(){
	printf("%d", find("salam\0", "dash salam\0"));
}