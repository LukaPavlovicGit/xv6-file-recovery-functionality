#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int
main(int argc, char *argv[])
{
	char *result = malloc(64*(sizeof(char)*(DIRSIZ+1)));
	int k=-1;
	
	if(argc == 1)
		k = lsdel(".",result);
	else if(argc == 2)
		k = lsdel(argv[1],result);
	
	
	if(k < 0)
		printf("Navedena putanja nije validna.\n");	
	else if(k == 0)
		printf("Nema obrisanih datoteka u navedenom direktorijumu.\n");
	else{

		int x=0, j=0, i=0;
		for( ; x<k ; i=(++x)*(DIRSIZ+1), j=0, printf("\n")){ // i-broj reda ; j-offset u i-tom redu

			while(result[i+j] != 0){
				printf("%c", result[i+j]);
				j++;		
			}
		}
	}
	free(result);
	exit();
}
