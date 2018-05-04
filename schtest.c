#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
	
	int pri;
	int opt = (int)argv[1][0];
	int i,k;
	const int loop = 43000;
	switch (opt) {
		case '1':
		    pri = 30;
		    break;
		case '2':
			pri = 20;
			break;
		case '3':
			pri = 10;
			break;
		default:
		    pri = 10;
		    break;
	}

	priority(pri);

    for (i = 0; i < loop; i++) {
    	asm("nop");
    	for (k = 0; k < loop; k++)
    		asm("nop");
    }
	exit();
}