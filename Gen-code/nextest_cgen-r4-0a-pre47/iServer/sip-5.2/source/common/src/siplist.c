
/******************************************************************************
 ** FUNCTION:
 **	 This file contains the implementations of the functions used to 
 **	 manipulate SipList structures. 
 ******************************************************************************
 **
 ** FILENAME:
 ** siplist.c
 **
 ** DESCRIPTION:
 **	This file implements all functions required to perform operations
 **     on the SipLists	
 **	
 **
 ** DATE			NAME			REFERENCE		REASON
 ** ----			----			--------		------
 ** 19/11/99		   Binu K S     		--			Original
 **
 **
 **	Copyright 1999, Hughes Software Systems, Ltd. 
 *****************************************************************************/

#include "portlayer.h"
#include "siplist.h"


/******************************************************************************
 ** FUNCTION: sip_listInit
 ******************************************************************************
 ** This function initializes the list structure passed to it.
 ** It sets the head and tail pointers to SIP_NULL and the size to zero.
 ** This funct also set the free function pointer in the list structure.
 ** Needs to be called before any other operations are performed on a list.
 ** Need not be called after DeleteAll before reusing the deleted list.
 *****************************************************************************/

SipBool sip_listInit
#ifdef ANSI_PROTO
	( SipList 		*list,
	  sip_listFuncPtr freefunc,
	  SipError 		*error )
#else
	( list, freefunc, error)
	  SipList 		*list;
	  sip_listFuncPtr freefunc;
	  SipError 		*error;  
#endif
{
 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }
 list->tail = list->head = SIP_NULL;
 list->size = 0;
 list->freefunc = freefunc;
 *error = E_NO_ERROR;
 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listAppend
 ******************************************************************************
 ** This function appends the pData passed to the siplist. The list maintains 
 ** a pointer to the pData passed and does not copy the pData. Hence pData must 
 ** not be freed after it has been appended to a siplist.
 *****************************************************************************/
SipBool sip_listAppend
#ifdef ANSI_PROTO
	( SipList 		*list,
	  void 			*pData,
	  SipError 		*error )
#else
	( list, pData, error )
	  SipList 		*list;
	  void 			*pData;
	  SipError 		*error;
#endif
{
 SipListElement *tmp, *it;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 tmp = (SipListElement *)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipListElement),error); 
 if(tmp==SIP_NULL)
 { 
  *error = E_NO_MEM;
  return SipFail;
 }
 tmp->pData = pData;
 tmp->next=SIP_NULL;
 
 if(list->head==SIP_NULL)
 {
  list->head = list->tail = tmp;
  list->size++;
  *error = E_NO_ERROR;
  return SipSuccess;
 }
 it = list->tail;
 it->next = tmp;
 list->tail = tmp;
 list->size++;
  *error = E_NO_ERROR;
 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listPrepend
 ******************************************************************************
 ** This function prepends the pData passed to the siplist. The list maintains 
 ** a pointer to the pData passed and does not copy the pData. Hence pData must 
 ** not be freed after it has been prepended to a siplist.
 *****************************************************************************/
SipBool sip_listPrepend
#ifdef ANSI_PROTO
	( SipList 		*list,
	  void 			*pData, 
	  SipError 		*error )
#else
	( list, pData, error )
	  SipList 		*list;
	  void 			*pData;
	  SipError 		*error;
#endif
{
 SipListElement *tmp;
 tmp = (SipListElement *)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipListElement),error); 

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 if(tmp==SIP_NULL)
 { 
  *error = E_NO_MEM;
  return SipFail;
 }
 tmp->pData = pData;
 tmp->next = list->head;
 if(list->tail==SIP_NULL) list->tail = list->head;
 list->head = tmp;
 list->size++;
  *error = E_NO_ERROR;
 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listGetAt
 ******************************************************************************
 ** This function gets the pData at the index specified. 
 ** The pointer to the pData is returned by the function. It must not be freed.
 ** Indexing begins from 0. 
 *****************************************************************************/

SipBool sip_listGetAt
#ifdef ANSI_PROTO
	( SipList 		*list,
	  SIP_U32bit 		position,
	  SIP_Pvoid 		*pData, 
	  SipError 		*error )
#else
	( list, position,pData, error )
	  SipList 		*list;
	  SIP_U32bit 		position;
	  SIP_Pvoid		*pData;
	  SipError 		*error;
#endif
{
	SipListElement *it;
	SIP_U32bit counter=0;

	if(list==SIP_NULL) 
	{
		*error = E_NO_EXIST;
		return SipFail;
	}
	
	if(list->head == SIP_NULL)
	{
		*error = E_NO_EXIST;
		return SipFail;
	}
	
	for(it=list->head;(it!=SIP_NULL)&&(counter!=position);it=it->next,counter++)
 	{}

	if((it==SIP_NULL)||(counter!=position)) 
	{
		*error = E_INV_INDEX;
		return SipFail;
	}
	else 
	{
		*pData = it->pData;
		*error = E_NO_ERROR;
		return SipSuccess;
	}
}

/******************************************************************************
 ** FUNCTION: sip_listSetAt
 ******************************************************************************
 ** This function sets the pData at the index specified. 
 ** It frees up pData that exists at the index before the set is invoked.
 ** The pData pointer passed to this function must not be freed.
 ** Index begins at 0.
 *****************************************************************************/

SipBool sip_listSetAt
#ifdef ANSI_PROTO
	( SipList 		*list,
	  SIP_U32bit 		position,
	  void 			*pData, 
	  SipError 		*error )
#else
	( list, position, pData, error )
	  SipList 		*list;
	  SIP_U32bit 		position;
	  void 			*pData;
	  SipError 		*error;
#endif
{
 SipListElement *it;
 SIP_U32bit counter=0;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }
 for(it=list->head;(it!=SIP_NULL)&&(counter!=position);it=it->next,counter++)
 {}
 if((it==SIP_NULL)||(counter!=position)) 
 {
  *error = E_INV_INDEX;
  return SipFail;
 }
 else 
 {
  if(list->freefunc!=SIP_NULL) list->freefunc(it->pData);
  it->pData = pData;
  *error = E_NO_ERROR;
  return SipSuccess;
 }
}

/******************************************************************************
 ** FUNCTION: sip_listSizeOf
 ******************************************************************************
 ** This function returns the number of elements in the list.
 *****************************************************************************/
SipBool sip_listSizeOf
#ifdef ANSI_PROTO
	( SipList 		*list, 
	  SIP_U32bit 		*size, 
	  SipError 		*error )
#else
 	( list, size, error )
	  SipList 		*list; 
	  SIP_U32bit 		*size; 
	  SipError 		*error; 
#endif
{

 if(list==SIP_NULL) 
 {
  *size = 0;
  *error = E_NO_EXIST;
  return SipFail;
 }
 *size = list->size;
 *error = E_NO_ERROR;
 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listInsertAt
 ******************************************************************************
 ** This function inserts pData at the index specified. 
 ** The pData passed is placed at the index given to the function.
 ** The pData pointer passed to this function must not be freed.
 ** Index begins at zero.
 *****************************************************************************/
SipBool sip_listInsertAt
#ifdef ANSI_PROTO
	( SipList 		*list, 
	  SIP_U32bit 		position, 
	  void 			*pData, 
	  SipError 		*error )
#else
	( list, position, pData, error )
	  SipList 		*list;
	  SIP_U32bit 		position;
	  void 			*pData;
	  SipError 		*error;
#endif
{
 SipListElement *tmp, *it;
 SIP_U32bit counter=0;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 if(position>(list->size)) 
 {
  *error = E_INV_INDEX;
  return SipFail;
 }
 if(position==list->size) return sip_listAppend(list,pData,error);
 if(position==0) return sip_listPrepend(list,pData,error);
 for(it=list->head;(it!=SIP_NULL)&&(counter!=position-1);it=it->next,counter++)
 {}
 tmp = (SipListElement *)fast_memget(NON_SPECIFIC_MEM_ID,sizeof(SipListElement),error); 
 if(tmp==SIP_NULL) 
 {
  *error = E_NO_MEM;
  return SipFail;
 }
 tmp->pData = pData;
 tmp->next = it->next;
 it->next = tmp; 
 list->size++;
  *error = E_NO_ERROR;
 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listDeleteAt
 ******************************************************************************
 ** This function removes the element at the index specified.
 ** It also frees up the memory allocated to the pData using the free function
 ** supplied in sip_listInit.
 ** Index begins at zero.
 *****************************************************************************/
SipBool sip_listDeleteAt
#ifdef ANSI_PROTO
	( SipList 		*list, 
	  SIP_U32bit 		position, 
	  SipError 		*error )
#else
 	( list, position, error )
	  SipList 		*list; 
	  SIP_U32bit 		position; 
	  SipError 		*error; 
#endif
{
 SipListElement *tmp, *it;
 SIP_U32bit counter=0;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }
 
 if(list->size==0)
 {
  *error = E_INV_INDEX;
  return SipFail;
 }

 if(position>(list->size-1))
 {
  *error = E_INV_INDEX;
  return SipFail;
 }
 if(position==0) 
 {
  tmp = list->head;
  list->head = list->head->next;
  if(list->head==SIP_NULL) list->tail=SIP_NULL;
  if (tmp!=SIP_NULL)
  {
  		if(list->freefunc!=SIP_NULL)
   			if (tmp->pData!=SIP_NULL) list->freefunc(tmp->pData);
  		fast_memfree(NON_SPECIFIC_MEM_ID,tmp,NULL);
  }
  list->size--;

  *error = E_NO_ERROR;
  return SipSuccess; 
 }
 for(it=list->head;(it!=SIP_NULL)&&(counter!=position-1);it=it->next,counter++)
 {}
 tmp = it->next;
 it->next = tmp->next;
 if (tmp!=SIP_NULL)
 {
 	if(list->freefunc!=SIP_NULL)
   		if (tmp->pData!=SIP_NULL) list->freefunc(tmp->pData);
  	fast_memfree(NON_SPECIFIC_MEM_ID,tmp,NULL);
 }
 if(tmp==list->tail)
	list->tail = it;
 list->size--;
  *error = E_NO_ERROR;

 return SipSuccess;
}

/******************************************************************************
 ** FUNCTION: sip_listForEach
 ******************************************************************************
 ** This function iterates through the list and invokes the function
 ** passed to it on every element in the list.
 *****************************************************************************/

SipBool sip_listForEach
#ifdef ANSI_PROTO
	( SipList 		*list,
	  sip_listFuncPtr func,
	  SipError 		*error )
#else
 	( list, func, error )
	  SipList 		*list;
	  sip_listFuncPtr func;
	  SipError 		*error; 
#endif
{
 SipListElement *it;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 for(it=list->head;it!=SIP_NULL;it=it->next)
  func(it->pData);
  *error = E_NO_ERROR;
  return SipSuccess;
}


/******************************************************************************
 ** FUNCTION: sip_listForEachWithData
 ******************************************************************************
 ** This function iterates through the list and invokes the function
 ** passed to it on every element in the list.
 *****************************************************************************/

SipBool sip_listForEachWithData
#ifdef ANSI_PROTO
	( SipList 		*list,
	  sip_listFuncPtrWithData func,
	  SIP_Pvoid           *pData,
	  SipError 		*error )
#else
 	( list, func, pData, error )
	  SipList 		*list;
	  sip_listFuncPtrWithData func;
	  SIP_Pvoid           *pData;
	  SipError 		*error; 
#endif
{
 SipListElement *it;

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 for(it=list->head;it!=SIP_NULL;it=it->next)
  func(it->pData,pData);
  *error = E_NO_ERROR;
  return SipSuccess;
}





/******************************************************************************
 ** FUNCTION: sip_listDeleteAll
 ******************************************************************************
 ** This function removes all elements from the list.
 ** It also frees up the memory allocated to the pData using the free function
 ** supplied in sip_listInit.
 ** The list can be reused without an Init after this funcion is called.
 *****************************************************************************/
SipBool sip_listDeleteAll
#ifdef ANSI_PROTO
	( SipList 		*list,
	  SipError 		*error )
#else
 	( list,error )
	  SipList 		*list;
	  SipError 		*error; 
#endif
{

 if(list==SIP_NULL) 
 {
  *error = E_NO_EXIST;
  return SipFail;
 }

 while(list->size!=0)
  if(sip_listDeleteAt(list,0,error)!=SipSuccess) return SipFail;
  *error = E_NO_ERROR;
 return SipSuccess;
}

