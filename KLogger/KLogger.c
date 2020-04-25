//
//  KLogger.c
//  KLogger
//
//  
//  Copyright © 2020 Anonymouz4. All rights reserved.
//

#include "KLogger.h"
#include <mach/mach_types.h>
#include <sys/kern_event.h>
#include <sys/kern_control.h>
#include <string.h>				//bzero
#include <libkern/libkern.h>	//printf

#define min(a, b) ((a) > (b))? (b): (a)
#define max(a, b) ((a) > (b))? (a): (b)

#define NEW_LOG_EVENT 1

int KLog(char *format, ...) {
	va_list args;
	va_start(args, format);
	char dataToWrite[1000]; vsnprintf((char*)dataToWrite, sizeof(dataToWrite), format, args);
	int ret = KLogT(dataToWrite, LOG_TYPE_DEBUG);
	va_end(args);
	return ret;
}

int KLogT(char* text, LOG_TYPE type) {
	
    errno_t status = KERN_FAILURE;
	
	u_int32_t ownVendorID = 0;//start
	kev_vendor_code_find(OwnVendor, &ownVendorID);

	//kernel event message
	struct kev_msg kEventMsg = {0};
	kEventMsg.vendor_code = ownVendorID;
	kEventMsg.kev_class = KEV_ANY_CLASS;
	kEventMsg.kev_subclass = KEV_ANY_SUBCLASS;
	kEventMsg.event_code = NEW_LOG_EVENT;

	
	//chunk pointer
    char* chunkPointer = text;
	uint sentStrLen = 0;
	LOG_MSG msg = {0};
	
	
	while(*chunkPointer != 0x0) {
		
		sentStrLen = min((u_int)strlen(chunkPointer), (maxLogStr - 1));
		
		bzero(&msg, sizeof(msg));
		msg.log_type = type;
		strncpy(msg.data, chunkPointer, sentStrLen);
		
        kEventMsg.dv[0].data_ptr = &msg;
		kEventMsg.dv[0].data_length = sizeof(msg);
        
        //broadcast msg to user-mode
        status = kev_msg_post(&kEventMsg);
        if(status != KERN_SUCCESS) {
            //err msg
        }
		
        //advance chunk pointer
		chunkPointer += sentStrLen;
    }
	
	return status;
}
