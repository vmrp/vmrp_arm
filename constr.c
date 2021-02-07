#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#define DSM_MAX_FILE_LEN 600

int32_t mrc_getLen(const char* filename)
{
	char fullpathname[DSM_MAX_FILE_LEN] = { 0 };
	struct stat s1;
	int ret;
	// sprintf(fullpathname,"%s%s",dsm_getPlatDir(),filename);
	ret = stat(filename, &s1);

	if (ret != 0)
		return -1;

	return s1.st_size;
}

int main(int argc, char const *argv[]) {
    FILE* fstream;
    FILE* fout;
    char msg[100] = "Hello!I have read this file.";
    if(argc>1){
        int32_t len = mrc_getLen(argv[1]);
fstream = fopen(argv[1], "at+");
    if (fstream == NULL) {
        printf("open file %s failed!\n",argv[1]);
        exit(1);
    } else {
        // printf("open file %s succeed!\n",argv[1]);
        char *buf = malloc(len);
        fout = fopen("out.txt","wb+");

        fread(buf, len, 1, fstream);
        for(int i=0;i<len;i++){
            // && buf[i]!='\n'
            if(buf[i]!='\r' ){
                printf("%c",buf[i]);
                fwrite(buf+i,1,1,fout);
            }
        }
        fclose(fout);
        fclose(fstream);
    }
    
    }
    else{
        printf("please input file\n");
    }
    
    return 0;
}