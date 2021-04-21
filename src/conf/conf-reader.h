#ifndef _CONF_READER_H_
#define _CONF_READER_H_

#define MAX 1024
#define MAX_FILE_PATH 256

#include <stdio.h>

struct conf_item
{
      char key[MAX];
      char value[MAX];
};

struct conf_reader
{
      char file_path[MAX_FILE_PATH];
      struct conf_item items[MAX];
      int item_count; 
};


int read_conf(struct conf_reader *reader, char* file_path, char *mode);

static int conf_parser(struct conf_reader *reader, FILE *fp);

int get_value(struct conf_reader *reader, char *key, char *value);

#endif // !_CONF_READER_H_ 