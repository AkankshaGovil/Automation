#include "osdconfig.h"
#include "nxosd.h"
#if HAVE_MKDIRP
#include <libgen.h>
#else
#include "nx_mkdirp.h"
#endif

#if HAVE_MKDIRP

int nx_mkdirp(const char *path, mode_t mode){
	
	return mkdirp(path, mode);
}
#else
// Fallback implementation for mkdirp

/*!
   \param array The array to be freed.
   \desc
   Frees an array of strings. Can be used for other data types that do not
     contain sub-pointers like structs. Last element in the array MUST be
     NULL.
*/
void array_string_free (char **array) {
	int i;
	if (!array) return;
	for (i=0; array[i]; i++) {
		free (array[i]);
	}
	free (array);
}


/*!
   \param str The string to search through.
   \param mat The string to match.
   \param rep The replacement string.
   \param allinst Set if allinstances of 'mat' are to be replaced.
   \retval "char *" Of the newly allocated string.
   \retval NULL On error.
   \desc
   Searches the string 'str' for text matching 'mat', and replaces it with
     the text 'rep'. If 'allinst' is set every instance of 'mat' will be
     replaced, otherwise only the first occurrence will be replaced.
*/
char *search_replace (char *str, char *mat, char *rep, int allinst) {
	size_t retlen, slen, matlen, replen, ittr=0;
	char *ret=NULL, *tmpret, *tmps1, *tmps2, *tmps3;

	if (!str) {
		return NULL;
	}
	if (!mat) {

		return NULL;
	}
	if (!rep) {

		return NULL;
	}

	slen=strlen (str);
	matlen=strlen (mat);
	replen=strlen (rep);

	if (!allinst) {
		if ((tmps1=strstr (str, mat))) {
			retlen=slen-matlen+replen;
			ret=scalloc (retlen+2);
			tmps2=tmps1+matlen;
			memmove (ret, str, tmps1-str);
			addtext (ret, rep);
			addtext (ret, tmps2);
			return ret;
		} else {
			return strdup (str);
		}
	}
	tmps3=str;
	for (tmps1=strstr (str, mat); tmps1; tmps1=strstr (tmps2, mat)) {
		ittr++;
		tmpret=scalloc (slen-(matlen*ittr)+(replen*ittr)+2);
		if (ret) {
			addtext (tmpret, ret);
			free (ret);
		}
		ret=tmpret;
		tmps2=tmps1+matlen;
		memmove (ret+strlen (ret), tmps3, tmps1-tmps3);
		addtext (ret, rep);
		tmps3=tmps2;
	}
	if (!ret) return strdup (str);
	tmpret=scalloc (strlen (ret)+ strlen (tmps3)+2);
	addtext (tmpret, ret);
	addtext (tmpret, tmps3);
	free (ret);
	ret=tmpret;
	return ret;
}



/*!
   \param string The string to be exploded.
   \param sep The separating character.
   \param pad A padding character to ignore.
   \retval "char **" Of the newly allocated array of strings.
   \retval NULL On error.
   \desc
   Explodes a delimited string into an array. Escaped literals are
     supported. Quotes are ignored so you should encode the delimiter even
     if its wrapped in quotes.
*/
char **array_string_explode (char *string, char sep, char pad) {
	char **array=NULL, **tmparray, *tmps1, *tmps2, *tmps3;
	char sepliteral[4], seprep[4];
	int startpos, entries=0, i, stripslashes=FALSE;

	if (!string) {
		return NULL;
	}
	if (!sep) {
		return NULL;
	}

	sepliteral[0]='\\';
	sepliteral[1]=sep;
	sepliteral[2]='\0';
	seprep[0]=sep;
	sepliteral[1]='\0';

	for (tmps2=string; *tmps2 == pad; tmps2++);
	startpos=tmps2-string;
	for (tmps1=strchr (tmps2, sep);
	     tmps1;
	     tmps1=strchr (tmps2, sep)) {
		if (*(tmps1-1) == '\\') {
			stripslashes=TRUE;
			tmps2=tmps1+1;
			continue;
		}

		tmparray=tncalloc (char *, entries+2);

		if (array) {
			for (i=0; array[i]; i++) {
				tmparray[i]=array[i];
			}
			free (array);
		}

		array=tmparray;
		array[entries]=scalloc (tmps1-tmps2+2);
		memmove (array[entries], tmps2, tmps1-tmps2);
		if (stripslashes) {
			tmps3=search_replace (array[entries], sepliteral, seprep, TRUE);
			free (array[entries]);
			array[entries]=tmps3;
			stripslashes=FALSE;
		}
		tmps2=tmps1+1;
		entries++;

		for (; *tmps2 == pad; tmps2++);

	}

	if (tmps2 != string+strlen (string) && *tmps2 != '\n') {

		tmps1=string+strlen (string);
		if (*tmps1 == '\n') tmps1--;
		tmparray=tncalloc (char *,entries+2);

		if (array) {
			int i;
			for (i=0; array[i]; i++) {
				tmparray[i]=array[i];
			}
			free (array);
		}

		array=tmparray;
		array[entries]=scalloc (tmps1-tmps2+2);
		memmove (array[entries], tmps2, tmps1-tmps2);
		if (stripslashes) {
			tmps3=search_replace (array[entries], sepliteral, seprep, TRUE);
			free (array[entries]);
			array[entries]=tmps3;
			stripslashes=FALSE;
		}
		entries++;
	}

	return array;
}


int nx_mkdirp (const char *pathname, mode_t mode) {
	int i, check;
	char **dirparts;
	char *dirstr;
	struct stat st;

	if (!pathname) {
		printf ("mkdirp(): Error: Argument 'pathname' is null.\n");
		return -1;
	}
	dirparts=array_string_explode ((char *)pathname, '/', ' ');
	if (!dirparts) {
		printf ("mkdirp(): Error: Could not parse 'pathname'.\n");
		return -1;
	}
	dirstr=scalloc (strlen (pathname)+4);
	for (i=1; dirparts[i]; i++) {
		addtext (dirstr, "/");
		addtext (dirstr, dirparts[i]);
		check=stat (dirstr, &st);
		if (!check) {
			if (!S_ISDIR (st.st_mode)) {
				errno=ENOTDIR;
				array_string_free (dirparts);
				free (dirstr);
				return -1;
			}
			continue;
		}
#ifndef WIN32
		check=mkdir (dirstr, mode);
#else
		check=mkdir (dirstr);
#endif
		if (check == -1) {
			array_string_free (dirparts);
			free (dirstr);
			return -1;
		}
	}
	array_string_free (dirparts);
	free (dirstr);

	return 0;
}

#endif/* HAVE_MKDIRP*/

