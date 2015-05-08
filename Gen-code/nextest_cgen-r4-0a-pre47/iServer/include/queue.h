
#ifndef _queue_h_
#define _queue_h_


/* What is a Q? */
typedef struct _queue_ * Q;

int qInit (Q *q, int size, int depth);
int qSend (Q q, void * item);
int qReceive (Q q, void ** item);
int qStatus (Q q);
int qItems (Q q);

#endif
