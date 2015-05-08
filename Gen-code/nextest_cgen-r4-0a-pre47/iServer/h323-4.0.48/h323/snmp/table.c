#ifdef __cplusplus
extern "C" {
#endif

#include <ra.h>
#include <snmpI.h>
#include <mib.h>


int searchByIndex(UINT8 *key,int kSize,tableT * table,int * location);
int  compareG(UINT8 *key,int kSize ,int pos,tableHandleT table);




tableT *  tableConstruct(int nElements,int keySize,int elementSize,tableGetDataT   get,RAECompare compare,
						 tableGetIndexT getIndex)
{
	tableT * table = (tableT *)malloc(sizeof(tableT));
	if (table==NULL)
		return NULL;
	table->size=0;
	table->keySize = keySize;
	table->get = get;
	table->compare= compareG;
		table->sortData = (int *)malloc(sizeof(sortDataHandleT)*nElements);
	table->getIndex = getIndex;
	if(table->sortData==NULL)
	{
		free(table);
		return NULL;
	}
	table->data = raConstruct(elementSize, nElements, NULL, "SNMP TABLE");
    raSetCompareFunc(table->data, compare);
	if(table->data==NULL)
	{
		free(table->sortData);
		free(table);
		return NULL;

	}
	return table;
}



void tableDestruct(tableT *table)
{
	if(table==NULL)
		return;
	raDestruct(table->data)	;	
	free(table->sortData);
	free(table);

}

/*************************************************************
 * tableAdd - add element (data) to the table 
 *************************************************************/
int tableAdd(IN tableT *table, IN RAElement data, OUT int* entry)
{
	int location=0,res,pos,ii;
    RAElement newElem;
	UINT8 * index;

	pos = raAdd(table->data, &newElem);
	if (pos >= 0)
	{
        memcpy((void*)newElem, data, raElemSize(table->data));
		if(table->size >0)
		{
			index= table->getIndex(newElem);

			/*
			  search if an element with such an index already exists in the table
			  res=0 means element is found
			*/
			res = searchByIndex(index,table->keySize,table,&location);
			if (!res)
			{
				/* delete just added element, because it's identical to already existed one
				 */
				raDeleteLocation(table->data, pos);
				return invalidIndex;
			}

			/* res >0 means that the key of the element is greater than all the keys in the table
			 * consequently we will add new element on the next location
			 */
			if(res > 0)
				location++;

			for (ii=table->size-1;ii>=location;ii--)
				table->sortData[ii+1] = table->sortData[ii];
		}
		table->sortData[location]=pos;
		*entry = pos+1;
		table->size++;
	}
    else
        return noMemoryRc;
	return 0;	
}

int tableSetDefault(tableT *table,int pos,h341ParameterName name,mibDataT *data,tableSetDefaultValT  setValue)
{
	RAElement element;
	if (setValue!=NULL)
	{
		element = raGet(table->data,pos-1);
		return setValue(element,name,data);
	}
	return 1;
}

UINT8 * tableFindIndex(tableT *table,void * data)
{
	int pos;
	RAElement element;
	pos = raFind(table->data,data);
	if(pos<0)
		return NULL;
	element = raGet(table->data,pos);
	return table->getIndex(element);

}

int  tableUpdateIndex(tableT *table,UINT8 * index,UINT8 *  newkey,int iSize)
{
	int res,location,pos,ii;
    UINT8 * key;
	RAElement element;

	res = searchByIndex(index,iSize,table,&location);
	if (!res)
	{   pos = table->sortData[location];
		for (ii=location+1;ii<table->size;ii++)		
			table->sortData[ii-1] = table->sortData[ii];
		table->size--;
        location = 0;
        if(table->size >0)
		{
			res = searchByIndex(newkey,table->keySize,table,&location);
			if (!res)
				return invalidIndex;
			if(res>0)
				location++;
			for (ii=table->size-1;ii>=location;ii--)
				table->sortData[ii+1] = table->sortData[ii];
		}
		table->sortData[location]=pos;
        element = raGet(table->data,pos);
        key = table->getIndex(element);
        memcpy(key,newkey,iSize);
        table->size++;
        return 0;
    }
    return res;
}

/*************************************************************
 * tableDelete - delete element defined by index from the table 
 *************************************************************/
int tableDelete(tableT *table,UINT8 * index,int iSize)
{

	int res,location,pos,ii;

	/*
	  search if an element with such an index already exists in the table
	  res=0 means element is found
	*/
	res = searchByIndex(index,iSize,table,&location);
	if (!res)
	{
		pos = table->sortData[location];
		raDeleteLocation(table->data, pos);
		for (ii=location+1;ii<table->size;ii++)		
			table->sortData[ii-1] = table->sortData[ii];
		table->size--;
		return 0;

	}
	return noSuchIndex;
}



/*******************************************************************************************
 * compareG - compare the 'key' with element defined by position 'pos' in the table 'hTable'
 * parameters:
 *    key    - key to compare
 *    kSize  - size of the key
 *    pos    - position of the 'comparable' element in the table
 *    htable - table wich contains the element
 * return:
 *     0 - if the key is identical with the key of the element in the table
 *     !0 - otherwise
 * comments - camparison is made by memcpy
 *******************************************************************************************/
int  compareG(UINT8 *key,int kSize ,int pos,tableHandleT hTable)
{
	int ii;
	RAElement element;
	tableT * table=(tableT *)hTable;
	ii = table->sortData[pos];
	element = raGet(table->data,ii);

	if (element!=NULL)
	{
/*		dPrintIndex(key,kSize,"add ");
		dPrintIndex(table->getIndex(element),kSize,"cmp ");*/

		return memcmp(key,table->getIndex(element),kSize);
	}
	return 1;	
}

/********************************************************************
 *searchByIndex - search element defined by 'key' in the 'table'
 *parameters:
 * input:
 *    key   - key that identified an element
 *    kSize - size of key
 *    table - table to search in
 *output 
 *    location - location of the element if found
 *               location where the element can be added
 *return: res=0  if element exists in the table
 *        res>0  the evaluated key is greater than the ones in the table
 *comment: search organized by "binary method"- 
           for n elements the range is allways cut by n/2 (limit>>=1) 
 ********************************************************************/
int searchByIndex(UINT8 *key,int kSize,tableT * table,int * location)
{

	int size,limit,res=0,beg=0;
	int pos=0;

	size = table->size;

	/* if the key has no length (kSize=0) there is nothing to search*/
	if (!kSize)
	{
		*location = 0;
		return 0;

	}
	for (limit = size; limit != 0; limit >>= 1) {
		pos =beg + (limit >> 1);
		res = table->compare(key,kSize,pos,(tableHandleT)table);
		*location = pos;
		
		/* if the key is found - search is done */
		if (res == 0)
			return res;

        /* if the key is greater then the one of the table's element - continue search forward */
		if (res > 0) {  
			beg = pos+1;
			limit--;
			
		} 
		/* if the key is smaller - continue search back - to smaller positions */
		
	}
		return res;
}


/*h.e - I think this is a great example that no comment is better than unclear one:
 
in case of indexsize  less than declared key size  element with the next index will be equal 
location if result of search less or equal 0,(it means that indexSize byte of key are equal or less
than indexSize byte of key of found element) and element with the next index will be equal location+1
*/

/*******************************************************************************
 * getNextIndex - get data about a next element after the one identified by 'index'
 *                Also get the next index and the size of the next index.
 * parameters
 * input:
 *   table - storage of requested data
 *   name  - (actually value) specifies parameter to retrieve
 *   index - index (key) of the parameter in case of several parameters with the same name 
 *           for example for each call there are will be a parameters with a same name defined by index
 * output:
 *   nextIndex - address to put the next index of the parameter (name)
 *   nextIndexSize - address to put the size of the next index
 * COMMENT: at cyclical requests about several indexes (for ex. about all connected calls)
 *          1st call of the function is without an index (indexSize=0).
 *          The function will return info about very 1st element in the table.
 *          Next call to the function will already be with a correct info about the
 *          previosly found element, and the function will return the data
 *          about the next element.
 *******************************************************************************/        
int getNextIndex (tableT * table ,h341ParameterName name,UINT8 * index, int indexSize,UINT8 *nextIndex,int *nextSize,
				  mibDataT *data)				
{
	int location,res;
	RAElement element ;
	if(!table->size)
		return lastRaw;
	if (table->keySize > indexSize)
	{
		
		res= searchByIndex(index,indexSize, table,&location);

		if (res > 0) 		
		{
			/* res>0 means that evaluated index is greater than all others in the table,
			 * consequently we have to define its location as 'next'.
			 */
			location++;
			if (location >= table->size)
				return lastRaw;	
		}

	}
	else 
	{
		res= searchByIndex(index,table->keySize, table,&location);
		 
		/* In case of res<0 we have an index that is less than evaluated one,
         * so 'location' points to the 'next index'.
	     *
		 * res=0 means that evaluated index is found
		 * to get info about next index we have to increase the location (get 'next').
		 */
		if (res <=0)
		{ 
		
			location++;
			if (location >= table->size)
				return lastRaw;	
			
		}
		
	}

	/* get an element */ 
	element = raGet(table->data,(int)table->sortData[location]); 
	
	/* get index of the element */
	memcpy(nextIndex, table->getIndex(element),table->keySize);
	
	/* get index size of the element */ 
	*nextSize=table->keySize;

	/* Eventually get element's data */
	if (data->valueSize!=0)
	{
		return table->get(element,name,data);
	}
	return 0;	
}


int getByIndex (tableT * table ,UINT8 * index, int indexSize, 
				h341ParameterName name,mibDataT *data)
{
		
	int location,res;
	RAElement element;
	
	if ((indexSize!=table->keySize)||(!table->size))
		return noSuchIndex;
	
	res= searchByIndex(index,table->keySize, table,&location);
	if (!res)
	{
		
		element = raGet(table->data,(int)table->sortData[location]); 
		table->get(element,name,data);
		return 0;
	}
	
	return noSuchIndex;
}


#ifdef __cplusplus
}
#endif
