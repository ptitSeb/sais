#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <dirent.h>

#include "cifm.h"
#pragma GCC optimize 0

bool ci_FileExists(const char* name) {
    if(!name) return true;
    if(name[0]=='\0') return true;
    if(strcmp(name,"/")==0) return true;
    
    struct stat info;
    int ret = stat(name, &info);
    return (ret==0);
}

struct ci_name_cache_t{
 char* source;
 char* dest;
};
#define CI_MAX 64
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
static ci_name_cache_t CacheRepo[CI_MAX];
static ci_name_cache_t *Cache[CI_MAX];

void CachePutFirst(int f) {
    if(!f) return;
    ci_name_cache_t *tmp = Cache[f];
    memmove(Cache+1, Cache+0, sizeof(ci_name_cache_t*)*f);
    Cache[0] = tmp;
}

const char* get_name(const char *name) {
    if(!name) return name;
    if(name[0]=='\0') return name;
    if(strcmp(name,"/")==0) return name;
    if(ci_FileExists(name)) {
        //printf("This one is good \"%s\"\n", name);
        return name;
    }
    if(strchr(name, '\\')) {
        static char tmp2[MAX_PATH];
        strcpy(tmp2, name);
        char* p = tmp2;
        while((p=strchr(p, '\\'))) *p='/';
        return get_name(tmp2);
    }
    if(name[strlen(name)-1]=='/') {
        static char tmp3[MAX_PATH];
        strcpy(tmp3, name);
        tmp3[strlen(name)-1] = '\0';
        static char tmp4[MAX_PATH];
        strcpy(tmp4, get_name(tmp3));
        strcat(tmp4, "/");
        return tmp4;
    }

    //printf("Try to fix \"%s\"\n", name);

    static int inited = 0;
    if(!inited) {
        for (int i=0; i<CI_MAX; i++) {
            CacheRepo[i].source = (char*)malloc(MAX_PATH);
            CacheRepo[i].dest =  (char*)malloc(MAX_PATH);
            CacheRepo[i].source[0] ='\0';
            CacheRepo[i].dest[0] ='\0';
            Cache[i] = &CacheRepo[i];
        }
        inited = 1;
    }
    // search in repo...
    for (int i=0; i<CI_MAX; i++)
        if(strcasecmp(name, Cache[i]->source)==0)
        if(ci_FileExists(Cache[i]->dest)) {
                //test if ok first, then return the value
                CachePutFirst(i);
                return Cache[0]->dest;
            }

    // split name / folder
    char *p = (char*)strrchr(name, '/');
    char *r = (p)?strndup(name, p-name):NULL;
    char *n = (p)?strdup(p+1):strdup(name);
    
    const char *new_r = get_name(r);
    
    if(!ci_FileExists(new_r)) {
        // fail to find the right path...
        free(r);
        free(n);
        return name;
    }
    // try to find a match for name now
	DIR *d;
    struct dirent *dir;
    static char tmp[MAX_PATH];
    d = opendir(new_r?new_r:".");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
            if (strcasecmp(dir->d_name, n)==0)
            {
                strcpy(tmp, new_r?new_r:"");
                strcat(tmp, new_r?"/":"");
                strcat(tmp, dir->d_name);
                if(ci_FileExists(tmp)) {
                    CachePutFirst(CI_MAX-1);
                    strcpy(Cache[0]->source, name);
                    strcpy(Cache[0]->dest, tmp);
                    free(r);
                    free(n);
                    return Cache[0]->dest;
                }
            }
		}
		closedir(d);
    }
    // build something...
    strcpy(tmp, new_r?new_r:"");
    strcat(tmp, new_r?"/":"");
    strcat(tmp, n);
// fail
    free(r);
    free(n);

    return tmp;
}


FILE* ci_fopen(const char* name, const char* mode)
{
    FILE *ret = fopen(name, mode);
    if(ret)
        return ret;
    const char* fixedname = get_name(name);
    ret = fopen(fixedname, mode);

    return ret;
}

const char* CI_FixName(const char* name)
{
    const char* fixedname = get_name(name);

    return fixedname;
}