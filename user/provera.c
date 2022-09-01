#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int
main(int argc, char *argv[])
{	
	int k;
	if(argc == 2)
		k = provera(argv[1]);
	
	exit();
}
