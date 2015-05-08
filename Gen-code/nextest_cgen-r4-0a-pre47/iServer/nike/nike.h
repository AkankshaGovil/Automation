#ifndef _NIKE_H_
#define _NIKE_H_

int adjust_enc_payload(SA *sa, short offset, short mul);
int oakley_process_preregistration_mode (isakmp_hdr *hdr, SA *sa);
int oakley_process_certcomm_mode (isakmp_hdr *hdr, SA *sa);
int initiate_preregistration_mode (SA *sa);
SA * initiate_certcomm(struct sockaddr_in *to_addr, struct sockaddr_in *src);
SA * initiate_preregistration(struct sockaddr_in *to_addr, struct sockaddr_in *src);


	

#endif
