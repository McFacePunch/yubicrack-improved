/** Yubicrack, a tool to brute force the access code of a Yubikey.
*
* Copyright © 2010 by Thomas Roth <code@leveldown.de>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <ykpers.h>
#include <yubikey.h>

/* Global variables, so that we can use them
 * in the bruteforce function. */
YK_KEY *yk;
YK_CONFIG *coreconfig;
int coreconfignum;

/* This prints the actual accesscode with an
 * individual message in front of it. */
void print_access_code(char* text, unsigned char* access_code) {
	printf("%s: %02x%02x%02x%02x%02x%02x\n",
		text,
		access_code[0],
		access_code[1],
		access_code[2],
		access_code[3],
		access_code[4],
		access_code[5]);
}
/* Iterate through only hex numbers values
 * This could take a 0.625*(loooooooooong time).*/
int bruteforceNumbers(unsigned char* t, int deep) {
    int id = 5 - deep;
    for (unsigned char counter = t[id]; counter <= 0x99; counter++) {
        if (counter % 16 >= 10) { counter += 6; }
        t[id] = counter;
        if (deep > 0) {
            if (bruteforceNumbers(t, deep - 1) == 0) return 0;
        } else {
            if (!yk_write_config(yk, coreconfig, coreconfignum, t)) {
                if (yk_errno != 3) {
                    printf("Error accessing the key: %d\n", yk_errno);
                    exit(1);
                } else if (!(t[id] % 0x50)) {
                    print_access_code("Fail", t);
                }
            } else {
                print_access_code("\aWin", t);
                return 0;
            }
        }
        //t[id] = 0;
        //return -1;
    }
    t[id] = 0;
    return -1;
}

int main(int argc, char** argv) {
    int i;
    char showmessage = 1;
    char *resume = NULL;

    for (i=1; i<argc; i++) {
        if(strcmp(argv[i], "-y") == 0) {
            showmessage = 0;
        } else {
            resume = argv[i];
        }
    }
    if(showmessage == 1) {
        puts("--------------------------------------------");
        puts("Hi! You're going to crack the access code of");
        puts("a Yubikey. As soon as the appropriate code  ");
        puts("is found, the AES key will be set to zeros.");
        puts("Brute forcing the code can take a very long ");
        puts("time, and with long I mean like more than a ");
        puts("year.");
        puts("(By the way you can bypass this message by  ");
        puts("passing the -y option to the program.) ");
        puts("--------------------------------------------");
        puts("Type \"start\" to continue.");

		char acknowledge[256];
		fgets(acknowledge, 256, stdin);
		if(strcmp(acknowledge, "start\n") != 0) {
			puts("Quitting.");
			return EXIT_SUCCESS;
		}
	} 

	yk = 0;
	unsigned char access_code[6];
	const char* aeshash="00000000000000000000000000000000";
	YKP_CONFIG *cfg = ykp_create_config();
	YK_STATUS *st = ykds_alloc();

	if(!yk_init()) {
		fputs("Failed to init Yubikey.\n", stderr);
		return EXIT_FAILURE;
	}
	if(!(yk = yk_open_first_key())) {
		fputs("No Yubikey found.\n", stderr);
		return EXIT_FAILURE;
	}
	if(!yk_get_status(yk,st)) {
		fputs("Failed to get status of the Yubikey.\n", stderr);
		return EXIT_FAILURE;
	}

	printf("Found Yubikey. Version: %d.%d.%d Touch level: %d\n",
		ykds_version_major(st),
		ykds_version_minor(st),
		ykds_version_build(st),
		ykds_touch_level(st));

	if(!ykp_configure_for(cfg, 1, st)) {
		printf("Can't set configuration to 1.\n");
		return EXIT_FAILURE;
	}
	if(ykp_AES_key_from_hex(cfg, aeshash)) {
		fputs("Bad AES key. WTF did you do to my source?", stderr);
		return EXIT_FAILURE;
	}

    coreconfig = ykp_core_config(cfg);
    coreconfignum = ykp_config_num(cfg);
    if (resume) {
        sscanf(resume, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
               &access_code[0],
               &access_code[1],
               &access_code[2],
               &access_code[3],
               &access_code[4],
               &access_code[5]);
        print_access_code("Resuming from", access_code);
        bruteforceNumbers(access_code, 5);
    } else{bruteforceNumbers(access_code, 5);}

	if(st) ykds_free(st);
	if(!yk_close_key(yk)) {
		fputs("Can't close Yubikey! What the hell are you doing over there?", stderr);
		return EXIT_FAILURE;
	}
	if(!yk_release()) {
		fputs("Can't release Yubikey.", stderr);
		return EXIT_FAILURE;
	}

	if(cfg) ykp_free_config(cfg);

	return EXIT_SUCCESS;
}
