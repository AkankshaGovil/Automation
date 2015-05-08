#ifndef _faxcontrol_h
#define _faxcontrol_h

#define FAX_CONTROL_PORT	10333

typedef struct
{
	unsigned char msg_type; /* Message Type */
#define FAX_FILE		0x01
#define FAX_FILE_RECEIVED	0x02
#define FAX_FILE_EXTRACTED	0x04
#define FAX_FILE_EXTRACT_ACK	0x08
	unsigned char unused[3];
	int fax_phone_numtype;
#define LUS_TYPE		0x1
#define VPNS_TYPE		0x2
	char fax_phone_number[PHONE_NUM_LEN];
	char src_email_addr[128];
	char dest_email_addr[128];
	char dest_file_name[128];
} FaxForwardMsg;

#endif /* _faxcontrol_h */
