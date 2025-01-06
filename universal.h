#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#ifndef UNIVERSAL_H
#define UNIVERSAL_H
#define WKP "mario"

struct server {
    int num_channels;
    char *channels[];
}

#endif
