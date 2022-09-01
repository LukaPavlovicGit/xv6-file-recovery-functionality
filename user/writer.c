#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"


void
writerr(char *name, int n)
{
	char buf[512];
	int fd; // file descriptor 
	struct stat st;
	
	//drugi argument oznacava mode (read,write,read i witer ili create); path moze biti i direktorijum, i u tom slucaju funkciju read za citanje cemo koristiti dirent strukturu (de)
	if((fd = open(name, 0x200)) < 0){	 // kreiramo ga (0x200)
		printf("writer: cannot create\n");
		return;
	}
	if((fd = open(name, 0x002)) < 0){ 	// otvaramo ga za citanje i pisanje (0x002);
		printf("writer: cannot open\n");
		return;
	}

	if(fstat(fd, &st) < 0){			// fstat cita inode iz file descriptora i atribute smesta u st (cita sve osim addr niza) 
		printf("writer: cannot stat\n");
		close(fd);
		return;
	}
	char c='a';
	int i,k=0;
	for(i=0 ; i<n ; ++i,++k){
		if(k==512){ 			// UPISUJEMO PO JEDAN BLOK U FAJL ; SVAKI BAJT U BLOKU JE ISTI
			if(write(fd,buf,k) != k)
				printf("writer: write");
			k = 0;
			if(c=='z') c='a';
			else ++c;
		}		
		buf[k]=c;
	}
	
	if(k>0 && (write(fd,buf,k) != k))
		printf("writer: write");
	
	close(fd);
}

int
main(int argc, char *argv[])
{	
	int i;
	if(argc == 3){
		i = atoi(argv[2]);
		writerr(argv[1],i); // OGRANICI VELICINU DRUGOG ARGUMENTA
	}
	
	exit();
}
