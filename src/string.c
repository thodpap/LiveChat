#include <stdio.h>

void clearArray(char *arr,int length){
	for(int i = 0; i < length; ++i){
		if(arr[i] == '\n'){
			arr[i] = '\0';
			break;
		}
	}
}
void clearStdout(){
	printf("\r%s","> ");
	fflush(stdout);
}
