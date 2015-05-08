#ifdef __cplusplus
extern "C" {
#endif



#include <rvinternal.h>
#include <psyntreeStackApi.h>
#include <pvaltree.h>
#include <emanag.h>
#include <msg.h>
#include <ms.h>
#include <q931.h>

static int msa,msaErr;

int qILog2(INT32 N)
{
    int i=-1;
    while(N) {i++;N>>=1;}
    return i;
}

typedef enum
{
    ROOT_LEVEL,
    MSG_LEVEL,
    OTHER_IES_LEVEL,
    IE_LEVEL,
    OCTET_LEVEL,
    FIELD_LEVEL
} qLevels;


int Q931EInter(HPST synH,int nodeId,INT32 fieldId ,HPVT valH,int vNodeId,int level,BYTE* buffer,int bit,int length)
{
    char *stringForPrint;
    int lengthBit=0;
    pstTagClass  tagClass;
    pstNodeType type;
    int numOfChildren;
    int tag=pstGetTag(synH,nodeId,&tagClass);
    if (tagClass==pstTagPrivate && tag!=emQ931)
    {
        int len, ret;
        msaPrintFormat(msa, "Private tag %d",    tag);
        ret = emEncodeExt(valH,vNodeId,buffer+bit/8,length,&len);
		if (ret < 0)
		{
			msaPrintFormat(msaErr, "Q931EInter: Private tag encoding error.");
			return RVERROR;
		}
        return bit+len*8;
    }

    if (tag != RVERROR)
    {
        /* See if we have to place a tag in front of the encoded element */
        if ((tagClass == pstTagEmpty) || (tagClass == pstTagApplication))
        {
            buffer[bit/8]=(BYTE)tag;
            bit += 8;
        }

        if (level == IE_LEVEL)
        {
            /* It's an information element - we have to add a length as well... */
            lengthBit = bit;
            bit+=(tagClass==pstTagApplication)?16:8;
        }
    }

    level++;
    type=pstGetNodeType(synH,nodeId);
    msaPrintFormat(msa, "Encoding %s: %s [%d].",(fieldId>=0)?pstGetFieldNamePtr(synH, fieldId):"(null)", pstGetTokenName(type), bit);
    numOfChildren=pstGetNumberOfChildren(synH,nodeId);
    switch(type)
    {
        case pstSet:
        {
            int i;
            for (i=0;i<numOfChildren;i++)
            {
                int childNodeId;
                pstChildExt child;
                pstGetChildExt(synH,nodeId,i+1,&child);

				if (pvtGetChild(valH,vNodeId,child.fieldId,&childNodeId) < 0)
				{
					if ((child.isOptional == TRUE) || (child.isDefault == TRUE))
						continue;
					else
					{
					      stringForPrint=pstGetFieldNamePtr(synH,child.fieldId);
						  msaPrintFormat(msaErr, "Q931EInter: non optional Child not found [%d]%s->%s.",
									     vNodeId,
										 nprn(stringForPrint),
										 nprn(pstGetFieldNamePtr(synH, child.fieldId)));
						  return RVERROR;
					}
				}
				bit=Q931EInter(synH,child.nodeId,child.fieldId,valH,childNodeId,level,buffer,bit,length);
		        if(bit<0) return bit;
			}
		}
        break;
        case pstSequence:
        {
            int i;
            for (i=0;i<numOfChildren;i++)
            {
                int childNodeId;
                pstChildExt child;
                pstGetChildExt(synH,nodeId,i+1,&child);
                if (pstGetIsExtended(synH,nodeId) && !(bit%8))
                {
                    buffer[bit/8]=0x80;
                    bit++;
                }
                if (pvtGetChild(valH,vNodeId,child.fieldId,&childNodeId)==RVERROR)
				{
					if (child.isExtended)
					{
						if(bit%8==1)
						{
				            bit--;
							buffer[bit/8-1]|=0x80;
						}
			            else
					    {
				            bit+=8-bit%8;
							/* ?????????????? */
						}
						break;
					}
			        else if ((child.isOptional == TRUE) || (child.isDefault == TRUE))
						continue;
					else
			        {
	                    stringForPrint=pstGetFieldNamePtr(synH,child.fieldId);
		                msaPrintFormat(msaErr, "Q931EInter: non optional Child not found [%d]%s->%s.",
			                vNodeId, nprn(stringForPrint),
                        nprn(pstGetFieldNamePtr(synH, child.fieldId)));
				        return RVERROR;
					}
				}
                bit=Q931EInter(synH,child.nodeId,child.fieldId,valH,childNodeId,level,buffer,bit,length);
                if(bit<0) return bit;
                if (i==numOfChildren-1) break;
                if (pstGetIsExtended(synH,nodeId) && !(bit%8)) buffer[bit/8-1]&=0x7f;
			}
		}
	    break;

        case pstSequenceOf:
        {
            /* SEQUENCE OF in Q931 is handled as if it's not there. We use it to allow several
               information elements with the same tag to be encoded/decoded together. */
            int childNodeId;
            int childSynNodeId;
            INTPTR childFieldId;

            /* Loop through the array and encode each element as if it was a fresh one out
               of the box. */
            childNodeId = pvtChild(valH, vNodeId);
            pvtGet(valH, childNodeId, &childFieldId, &childSynNodeId, NULL, NULL);

            while (childNodeId >= 0)
            {
                /* We go and encode our internal structures without increasing the level since we
                   don't want the SEQUENCE OF structure to show up in the encoding */
                bit = Q931EInter(synH, childSynNodeId, childFieldId, valH, childNodeId, level-1, buffer, bit, length);
                if (bit < 0)
                    return bit;

                /* Get the next one */
                childNodeId = pvtBrother(valH, childNodeId);
            }

            /* Make sure we don't think we're in the same length, since it causes a length
               value to be written on top of the protocol discriminator in Q931 */
            level++;
        }
        break;

        case pstChoice:
	    {
		    int childNodeId=pvtChild(valH,vNodeId);
			int childSynNodeId;
            INTPTR fieldid;

	        if(childNodeId==RVERROR)
		    {
			    stringForPrint=pstGetFieldNamePtr(synH, fieldId);
				msaPrintFormat(msaErr, "perEncodeChoice: Value node does not exist. [%s]",
							   nprn(stringForPrint));
		        return RVERROR;
			}

	        pvtGet(valH,childNodeId,&fieldid,&childSynNodeId,NULL,NULL);

		    bit=Q931EInter(synH,childSynNodeId,fieldId,valH,childNodeId,level,buffer,bit,length);
            if(bit<0) return bit;
        }
        break;
        case pstInteger:
	    {
		    UINT32 value;
			int to , from, bits;
	        pstGetNodeRange( synH, nodeId, &from, &to);
		    bits=qILog2(to-from)+1;
			pvtGet(valH,vNodeId,NULL,NULL,(INT32*)&value,NULL);
			if (bits<=8)  {buffer[bit/8]&=~(((1<<bits)-1)<<(8-(bit%8)-bits));buffer[bit/8]|=value<<(8-(bit%8)-bits);}
	        else if (bits==16)
		    {
				buffer[bit/8]=(BYTE)(value>>8);
		        buffer[bit/8+1]=(BYTE)value;
			}
	       bit+=bits;
		}
	    break;
        case pstIA5String:
        case pstOctetString:
	    {
		    UINT32 strLength;
			if (bit%8) {bit+=7; bit-=bit%8;}
	        pvtGet(valH,vNodeId,NULL,NULL,(INT32*)&strLength,NULL);
		    pvtGetString(valH,vNodeId,(INT32)strLength,(INT8*)&(buffer[bit/8]));
			bit+=((int)strLength)*8;
	    }
	    break;
        case pstNull: 
			if(bit-lengthBit==8) bit-=8;
            return bit;
	    default: 
			break;
    }
    if (level==IE_LEVEL+1)
      /* if (bit-lengthBit>8) */
    {
        if (tagClass==pstTagApplication)
        {
            buffer[lengthBit/8]=(unsigned char)(((bit-lengthBit)/8-2)>>8);
            buffer[lengthBit/8+1]=(unsigned char)((bit-lengthBit)/8-2);
        }
        else buffer[lengthBit/8]=(unsigned char)((bit-lengthBit)/8-1);
    }
    /*  else bit-=8;*/

  return bit;
}




int Q931Encode(HPVT valH,int vNodeId,BYTE* buffer,int length,int*encoded)
{
    HPST synH=pvtGetSynTree(valH, vNodeId);
    int nodeId;
    pvtGet(valH, vNodeId, NULL, &nodeId, NULL, NULL);
    *encoded=Q931EInter(synH,nodeId,-1,valH,vNodeId,ROOT_LEVEL,buffer,0,length*8);
    if(*encoded<0)
      return(*encoded);
    else
	{
	  *encoded /= 8;
      return 0;
	}
}


int Q931DInter(HPST synH,int nodeId,INT32 fieldId,HPVT valH,int vNodeId,int level,BYTE* buffer,int bit,int length)
{
    int newBit, ret;
    int tag = buffer[bit/8];
    pstTagClass  tagClass;
    pstNodeType type;
    int numOfChildren;
    int stag;

    type = pstGetNodeType(synH, nodeId);
    if (type != pstSequenceOf)
    {
        /* "Regular node" - get the tag from it */
        stag = pstGetTag(synH, nodeId, &tagClass);
    }
    else
    {
        /* It's a SEQUENCE OF - the tag is one step down */
        stag = pstGetTag(synH, pstGetNodeOfId(synH, nodeId), &tagClass);
    }

    if (nodeId != RVERROR)
    {
        if (tagClass == pstTagPrivate && stag != emQ931)
        {
            int len;
            ret = emDecodeExt(valH, vNodeId, fieldId, buffer + bit/8, (length/8) - (bit/8), &len);
            if (ret >= 0)
                return bit + len*8;
            else
                return ret;
        }
        if ((tagClass == pstTagEmpty || tagClass == pstTagApplication) &&  stag != RVERROR)
        {
            if (tag == stag)
                bit += 8;
            else
                return bit;
        }
    }
    else
        bit += 8;
    newBit = bit;
    if ((level == IE_LEVEL) && !(tag & 0x80))
    {
        int ieLen;
        int lenLen;
        if (nodeId != RVERROR && tagClass == pstTagApplication)
        {
            lenLen = 16;
            ieLen = (buffer[bit/8]*256 + buffer[bit/8 + 1])*8;
        }
        else
        {
            lenLen = 8;
            ieLen = buffer[bit/8]*8;
        }
        bit += lenLen;
        newBit = ieLen + bit;
    }

    level++;

    if (nodeId != RVERROR)
    {
        int i;
        int oldBit;
        int vcNodeId=vNodeId;
        pstChildExt child;

        numOfChildren=pstGetNumberOfChildren(synH,nodeId);

        switch (type)
        {
            case pstSet:
            {
                int searchedTag = -1;
                int lastTag;
                BOOL firstRun = TRUE;

                if (fieldId!=RVERROR)
                if ((vcNodeId = pvtAdd(valH, vNodeId, fieldId, 0, NULL, NULL)) < 0)
                    return vcNodeId;
                pstGetChildExt(synH, nodeId, 1, &child);
                i = 0;

                while (bit < length)
                {
                    lastTag = searchedTag;
                    searchedTag = buffer[bit/8];
                    if (searchedTag < lastTag)
                    {
                        /* Make sure the iterator starts all over again - we must search it all */
                        firstRun = TRUE;
                        pstGetChildExt(synH, nodeId, 1, &child);
                        searchedTag = -1;
                        i = 0;
                    }
                    else if (searchedTag == lastTag)
                    {
                        if (pstGetNodeType(synH, child.nodeId) != pstSequenceOf)
                        {
                            /* Oh - same tag twice - let's see if our application is ready for this one */
                            /* If it's ready, then it will have a SEQUENCE OF field right after the current
                               one, which will have the same tag */
                            pstChildExt nextChild;
                            int childRes;
                            childRes = pstGetChildExt(synH, nodeId, i+2, &nextChild);
                            if ((childRes >= 0) && (pstGetNodeType(synH, nextChild.nodeId) == pstSequenceOf))
                            {
                                if (pstGetTag(synH, pstGetNodeOfId(synH, nextChild.nodeId), NULL) == searchedTag)
                                {
                                    /* Definition has a SEQUENCE OF with the same tag value for the elements
                                       let's use them */
                                    child = nextChild;
                                    i++;
                                }
                            }
                        }

                        /* If we didn't use the SEQUENCE OF, then we just rewrite the current value,
                           as was the case in previous versions */
                    }
                    else
                    {
                        if (firstRun == FALSE)
                        {
                            /* Skip the current field - we don't really have to check it */
                            pstGetChildExt(synH, nodeId, i+2, &child);
                            i++;
                        }
                        else
                            firstRun = FALSE; /* We should make sure to check the first field */
                    }

                    /* Start searching for the tag and decode it while we're at it */
                    while (i < numOfChildren)
                    {
                        oldBit = bit;
                        bit = Q931DInter(synH, child.nodeId, child.fieldId, valH, vcNodeId, level, buffer, bit, length);
                        if (bit < 0)
                        {
                            /* Had an error */
                            return bit;
                        }
                        if (bit != oldBit)
                        {
                            /* We actually decoded it - we can get out of this while loop */
                            break;
                        }

                        /* Try the next one */
                        pstGetChildExt(synH, nodeId, i+2, &child);
                        i++;
                    }

                    if (i == numOfChildren)
                    {
                        /* Didn't find this tag - we should decode it and skip it */
                        bit = Q931DInter(synH, RVERROR, RVERROR, valH, RVERROR, level, buffer, bit, length);
                        if (bit < 0)
                            return  bit;

                        /* We want to start the search again so the tags we're going to see next
                           are going to be handled (iter==NULL) */
                        firstRun = TRUE;
                        pstGetChildExt(synH, nodeId, 1, &child);
                        searchedTag = -1;
                        i = 0;
                    }
                }

                /* No real value: pvtSet(valH, vcNodeId, fieldId, pvtNumChilds(valH, vcNodeId), NULL); */
            }
            break;
            case pstChoice:
            {
                if (fieldId!=RVERROR)
                if ((vcNodeId = pvtAdd(valH, vNodeId, fieldId, 0, NULL, NULL)) < 0)
                    return vcNodeId;
                for (i = 0; (i < numOfChildren) && ((bit < newBit) || (level != IE_LEVEL + 1)); i++)
                {
                    oldBit = bit;
                    pstGetChildExt(synH, nodeId, i + 1, &child);
                    bit = Q931DInter(synH, child.nodeId, child.fieldId, valH, vcNodeId, level, buffer, bit, length);
                    if (bit < 0)
                        return bit;
                    if (bit != oldBit)
                    {
                        pvtSet(valH, vcNodeId, fieldId, child.fieldId, NULL);
                        break;
                    }
                }
                if (i == numOfChildren)
                    return RVERROR;
            }
            break;
            case pstSequence:
            {
                BOOL extended = FALSE;
                BOOL extendedNode=pstGetIsExtended(synH,nodeId);
                if (fieldId!=RVERROR)
                if ((vcNodeId = pvtAdd(valH, vNodeId, fieldId, 0, NULL, NULL)) < 0)
                    return vcNodeId;
                length = (newBit == bit) ? length:newBit;
                for (i = 0; (i < numOfChildren) && ((bit < newBit) || (level != IE_LEVEL + 1)); i++)
                {
                    pstGetChildExt(synH, nodeId, i + 1, &child);
                    if (extendedNode && !(bit%8))
                    {
                        extended=!(buffer[bit/8]&0x80);
                        bit++;
                    }
                    bit = Q931DInter(synH, child.nodeId, child.fieldId, valH, vcNodeId, level, buffer, bit, length);
                    if (bit < 0)
                        return bit;
                    if (extendedNode && !(bit%8) && !extended)
                        break;
                }
                pvtSet(valH, vcNodeId, fieldId, pvtNumChilds(valH, vcNodeId), NULL);
                if (extendedNode  && extended)
                    while (!(buffer[bit/8]&0x80)) bit += 8;
            }
            break;

            case pstSequenceOf:
            {
                /* Seems like we should simulate receiving several information elements */
                int vSeqOfNode;

                /* Let's make sure first that we have the SEQUENCE OF in the PVT */
                vSeqOfNode = pvtAdd(valH, vNodeId, fieldId, 0, NULL, NULL);
                if (vSeqOfNode < 0)
                    return vSeqOfNode;

                /* Now let's add a new last element to this array by decoding the field - make sure
                   we go back in the buffer and up in level to simulate proper type and position. */
                bit = Q931DInter(synH, pstGetNodeOfId(synH, nodeId), -800, valH, vSeqOfNode, level-1, buffer, bit-16, length);
            }
            break;

            case pstInteger:
            {
                UINT32 value = 0;
                int to , from, bits;
                pstGetNodeRange( synH, nodeId, &from, &to);
                bits=qILog2(to-from)+1;

                if (bits <= 8)
                    value = (buffer[bit/8] >> ((8 - (bit%8)) - bits))&((1 << bits) - 1);
                else
                    if (bits == 16)
                        value = (buffer[bit/8] << 8) + buffer[bit/8 + 1];
                if ((ret = pvtAdd(valH, vNodeId, fieldId, (INT32)value, NULL, NULL)) < 0)
                    return ret;
                bit += bits;
            }
            break;
            case pstNull:
            {
                if ((ret = pvtAdd(valH, vNodeId, fieldId, 0, NULL, NULL)) < 0)
                    return ret;
            }
            break;

            case pstIA5String:
            case pstOctetString:
            {
                UINT32 strLength;
                char buff[256];
                strLength = (((level != IE_LEVEL + 1) ? length:newBit) - bit)/8;
                if (bit%8)
                {
                    bit += 7;
                    bit -= bit%8;
                }
                strLength = min(strLength, 255);
                memcpy(buff, &(buffer[bit/8]), (int)strLength);
                buff[strLength] = 0;
                if ((ret = pvtAdd(valH, vNodeId, fieldId, (INT32)strLength, buff, NULL)) < 0)
                    return ret;
            }
            break;
        default:
            break;
        }
    }
    if (level == IE_LEVEL + 1)
    {
        bit = newBit;
    }

    return bit;
}


int Q931Decode(HPVT valH,int vNodeId,INT32 fieldId,BYTE* buffer,int length,int *decoded)
{
    HPST synH=pvtGetSynTree(valH, vNodeId);
    int nodeId,ret;
    pvtGet(valH, vNodeId, NULL, &nodeId, NULL, NULL);
    ret=Q931DInter(synH,nodeId,fieldId,valH,vNodeId,ROOT_LEVEL,buffer,0,length*8);
    if(ret<0)
        return ret;

    *decoded=ret/8;

    return 0;
}



int q931Install(void)
{
    emTypeOfEncoding eSys;
    eSys.Encode = Q931Encode;
    eSys.Decode = Q931Decode;
    msa = msaRegister(0, "Q931", "Q931 Encoder/Decoder");
    msaErr = msaRegister(0, "Q931ERR", "Q931 Errors");
    return emSetEncoding(emQ931,&eSys);
}

#ifdef __cplusplus
}
#endif
