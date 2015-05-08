/**
 * this reads a config file in the following format:
 * 			key=value
 * and returns the values read
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "srvrlog.h"
#include "cylink.h"  /* need the stupid TRUE and FALSE */

/**
 * takes a given line in "key=value" format and seperates the key and value and
 * stores them in the given buffers
 *
 * @param line the string in "key=value" format
 * @param key the buffer to store the 'key' part of the string
 * @param keyLen the length of the 'key' buffer
 * @param value the buffer to store the 'value' part of the string
 * @param valueLen the length of the 'value' buffer
 *
 * @return -1 for any error, or the length of the value string
 */
static int keyValue (char *line, char *key, int keyLen, char *value, int valueLen)
{
   int index, len;

   memset(key, 0, keyLen);
   memset(value, 0, valueLen);

   len = strlen(line);
   if (len == 0)
	  return 0;

   for (index = 0; index < len && line[index] != '='; index++);
   if (index == len)
	  return -1;  /* no '=' found */

   memcpy(key, line, index);

   if ((index+1) < len)
	  memcpy(value, &line[index+1], len-index-1);

   return strlen(value);
}

/**
 * reads one line at a time or the 'size' bytes, i.e., either read until a '\n'
 * character is encountered or 'size' bytes are read
 *
 * @param fp the stream to read from
 * @param line the buffer to store the data read
 * @param size the size of the buffer
 *
 * @return the bytes read, or -1 for end of stream
 */
static int readLine (FILE *fp, char *line, int size)
{
   int index;
   char c;
   int errno;

   memset(line, 0, size);

   for (index = 0; index < size; index++)
   {
	  if (fread(&c, 1, 1, fp) != 1)
	  {
		 if (feof(fp))
		 {
			// last line?
			if (index > 0)
			   return index;
			return -1;
		 }

		 if ((errno = ferror(fp)))
		 {
            char * errmsg = strerror( errno );

            if ( errmsg )
            {
			    NETDEBUG(MFCE, NETLOG_DEBUG2, ("Error reading file [%d]: %s\n",
                        errno, errmsg ));
            }
            else
            {
			    NETDEBUG(MFCE, NETLOG_DEBUG2, ("Error reading file [%d]\n",
                        errno ));
            }

			return -1;
		 }
	  }

	  // have we gotten the line delineator?
	  if (c == '\n')
		 return index;

	  line[index] = c;
   }

   return index;
}


/**
 * reads the config file specified, and extracts the values
 *
 * @param file the full path name of the file to be read
 * @param numKeys the number of keys the caller is interested in
 * @param keys the array containing the key strings
 * @param values a 2-dimensional array to store the values. the caller should provide
 * the buffer space of size [numKeys][valueLen]. if no corresponding key is found, the
 * value entry will be an empty string.
 * @param valueLen the max size of the value field
 *
 * @return TRUE if everything was read correctly, FALSE if some error happens
 */
int ReadConfigFile (char *file, int numKeys, char ** keys, char values[][], int valueLen)
{
   FILE *fp;
   char key[128] = {0};
   char value[valueLen];
   char line[128+valueLen];
   int i, lineLen;

   if ((fp = fopen(file, "r")) == NULL)
   {
      char * errmsg = strerror(errno);
	  NETERROR(MFCE, ("Cannot open %s: %s\n", file, errmsg ));
	  return FALSE;
   }

   lineLen = 128+valueLen;
   /* initialize all values to empty string */
   for (i = 0; i < numKeys; i++)
	  memset(&((char *)values)[i*valueLen], 0, valueLen);

   while (readLine(fp, line, lineLen) != -1)
   {
	  if (strlen(line) == 0 || line[0] == '#')
		 continue;

	  if (keyValue(line, key, 128, value, valueLen) == -1)
	  {
		 NETDEBUG(MFCE, NETLOG_DEBUG2, ("ignoring \"%s\": not a key=value pair\n", line));
		 continue;
	  }

	  /* store the value into the appropriate spot in the array */
	  for (i = 0; i < numKeys; i++)
	  {
		 if (strcmp(key, keys[i]) == 0)
		 {
			memcpy(&((char *)values)[i*valueLen], value, valueLen);
			break;
		 }
	  }

	  if (i == numKeys)
	  {
		 NETDEBUG(MFCE, NETLOG_DEBUG4, ("line '%s' ignored from config file %s\n", line, file));
	  }
   }

   fclose(fp);

   return TRUE;
}
