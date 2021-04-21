
#include "../nose.h"
#include "conf-reader.h"

#include <stdlib.h>
#include <string.h>

int 
read_conf(struct conf_reader *reader, char* file_path, char *mode)
{
      if (reader == NULL || file_path == NULL || mode == NULL) return (ERROR);

      FILE *fp = NULL;

      if ((fp = fopen(file_path, mode)) == NULL)
            return (ERROR);
      
      reader->item_count = 0;
      strncpy(reader->file_path, file_path, strlen(file_path));

      return (conf_parser(reader, fp));
}

static int 
conf_parser(struct conf_reader *reader, FILE *fp)
{
      char tmp[MAX] = {0};
      struct conf_item item;
      int split_pos = 0;
      while(!feof(fp))
      {
            memset(tmp, 0, MAX);
            memset(&item, 0, sizeof(struct conf_item));
            fgets(tmp, MAX, fp);
            if (!strlen(tmp)) continue;
            // TODO
            for (split_pos = 0; split_pos < strlen(tmp) && tmp[split_pos] != '='; split_pos++);
            strncpy(item.key, tmp, split_pos);
            strncpy(item.value, tmp + split_pos + 1, strlen(tmp) - split_pos - 2); // \r\n

            memcpy(reader->items + reader->item_count++, &item, sizeof(struct conf_item));

      }

      fclose(fp);
      return (OK);
}

int 
get_value(struct conf_reader *reader, char *key, char *value)
{
      if (key == NULL || reader == NULL || value == NULL) return (ERROR);

      for (int i = 0; i < reader->item_count; i++)
      {
            if (strcmp(reader->items[i].key, key) == 0)
            {
                  strncpy(value, reader->items[i].value, strlen(reader->items[i].value));
                  return (OK);
            }     
      }
       
      return (FAILED);
}