/* $Id: dbi_sql.h,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
 *
 * Copyright (c) 1997,1998,1999  Tim Bunce  England
 *
 * See COPYRIGHT section in DBI.pm for usage and distribution rights.
 */


/* Some core SQL CLI standard (ODBC) declarations		*/
#ifndef SQL_SUCCESS	/* don't clash with ODBC based drivers	*/

/* Standard SQL datatypes (ANSI/ODBC type numbering)		*/
#define	SQL_ALL_TYPES		0
#define	SQL_CHAR		1
#define	SQL_NUMERIC		2
#define	SQL_DECIMAL		3
#define	SQL_INTEGER		4
#define	SQL_SMALLINT		5
#define	SQL_FLOAT		6
#define	SQL_REAL		7
#define	SQL_DOUBLE		8
#define SQL_DATE		9	/* SQL_DATETIME in CLI!	*/
#define SQL_TIME		10
#define SQL_TIMESTAMP		11
#define	SQL_VARCHAR		12

/* Other SQL datatypes (ODBC type numbering)			*/
#define SQL_LONGVARCHAR		(-1)
#define SQL_BINARY		(-2)
#define SQL_VARBINARY		(-3)
#define SQL_LONGVARBINARY	(-4)
#define SQL_BIGINT		(-5)	/* too big for IV	*/
#define SQL_TINYINT		(-6)

/* Support for Unicode and SQL92 */
#define SQL_BIT                 (-7)
#define SQL_WCHAR               (-8)
#define SQL_WVARCHAR            (-9)
#define SQL_WLONGVARCHAR        (-10)


/* Main return codes						*/
#define	SQL_ERROR			(-1)
#define	SQL_SUCCESS			0
#define	SQL_SUCCESS_WITH_INFO		1
#define	SQL_NO_DATA_FOUND		100

#endif	/*	SQL_SUCCESS	*/

/* Handy macro for testing for success and success with info.		*/
/* BEWARE that this macro can have side effects since rc appears twice!	*/
/* So DONT use it as if(SQL_ok(func(...))) { ... }			*/
#define SQL_ok(rc)	((rc)==SQL_SUCCESS || (rc)==SQL_SUCCESS_WITH_INFO)


/* end of dbi_sql.h */
