#ifndef __READCONFIG_H
#define __READCONFIG_H


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
extern int ReadConfigFile (char *file, int numKeys, char ** keys, char values[][], int valueLen);

#endif
