#include <stdio.h>
#include <string.h>
#include <bsdconv.h>

int main(int argc, char *argv[]){
	char *in=strdup("utf-8:utf-8,ascii");
	char *out;
	char *expect;

	expect="UTF-8:UPPER:UTF-8,ASCII";
	out=bsdconv_insert_phase(in, "upper", INTER, 1);
	if(strcmp(expect, out)){
		printf("Test failed at bsdconv_insert_phase\nexpect: %s\nresult: %s\n", expect, out);
		return 1;
	}
	free(in);

	in=out;
	expect="UTF-8:FULL:UTF-8,ASCII";
	out=bsdconv_replace_phase(in, "full", INTER, 1);
	if(strcmp(expect, out)){
		printf("Test failed at bsdconv_replace_phase\nexpect: %s\nresult: %s\n", expect, out);
		return 1;
	}
	free(in);

	in=out;
	expect="UTF-8:FULL:UTF-8,BIG5";
	out=bsdconv_replace_codec(in, "big5", 2, 1);
	if(strcmp(expect, out)){
		printf("Test failed at bsdconv_replace_codec\nexpect: %s\nresult: %s\n", expect, out);
		return 1;
	}
	free(in);

	in=out;
	expect="UTF-8,ASCII:FULL:UTF-8,BIG5";
	out=bsdconv_insert_codec(in, "ascii", 0, 1);
	if(strcmp(expect, out)){
		printf("Test failed at bsdconv_insert_codec\nexpect: %s\nresult: %s\n", expect, out);
		return 1;
	}
	free(in);

	printf("API tests passed\n");
	free(out);
	return 0;
}