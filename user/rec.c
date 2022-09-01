#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"


int
main(int argc, char *argv[])
{	
	int k=1;
	if(argc==1)
		printf("Nije naveden parametar na komandnoj liniji.\n");
	else if(argc==2)
		k=rec(argv[1]);
	else
		printf("Usage: rec <path>\n");
	

	switch(k){
	case  0 :
		printf("Uspesan oporavak.\n");
		break;

	case -1 :
		printf("Roditeljski direktorijum za navedenu datoteku nije validan.\n");
		break;	

	case -2 :
		printf("Ne postoji obrisana datoteka sa navedenim nazivom.\n");
		break;

	case -3 :
		printf("Inode datoteke je iskoriscen za nesto drugo.\n");
		break;

	case -4 :
		printf("Neki blok datoteke je iskoriscen za nesto drugo.\n");
		break;

	default :
		break;

	}
		
	exit();
}
