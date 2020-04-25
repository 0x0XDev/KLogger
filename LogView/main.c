//
//  main.c
//  KLogger
//
//
//  Copyright © 2020 Anonymouz4. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/kern_event.h>

#include <mach-o/dyld.h>


char *log_types[] = {"INFO","DEBUG"};

int fd = -1;

void disconnectFromKernel() {
    printf("[\033[0;32mo\033[0m] Disconnecting from kernel...\n");
    shutdown(fd, SHUT_RDWR);
	exit(0);

}

int main(int argc, const char * argv[]) {
	
	fd = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
	
	struct kev_vendor_code vendorCode = {0};

	strncpy(vendorCode.vendor_string, OwnVendor, KEV_VENDOR_CODE_MAX_STR_LEN);

	ioctl(fd, SIOCGKEVVENDOR, &vendorCode);
	
	struct kev_request kevRequest = {0};
	kevRequest.vendor_code = vendorCode.vendor_code;
	kevRequest.kev_class = KEV_ANY_CLASS;
	kevRequest.kev_subclass = KEV_ANY_SUBCLASS;

	//tell kernel what we want to filter on
	int ret = ioctl(fd, SIOCSKEVFILT, &kevRequest);
	if (ret) { printf("[\033[0;31mx\033[0m] Fail to connect to kernel... (%d)\n",ret); return -1; }
    printf("[\033[0;32mo\033[0m] Connected to kernel. Waiting for log...\n");
	
	signal(SIGTERM, disconnectFromKernel);
    signal(SIGINT, disconnectFromKernel);
	
	
	char kextMsg[KEV_MSG_HEADER_SIZE + sizeof(LOG_MSG)] = {0};
	struct kern_event_msg* kernEventMsg;
	ssize_t bytesReceived = -1;
	
	while(1) {
		
		bzero(&kextMsg, sizeof(kextMsg));
		bytesReceived = recv(fd, kextMsg, sizeof(kextMsg), 0);
		
		kernEventMsg = (struct kern_event_msg*)kextMsg;
        
        //sanity check
        // ->make sure data recv'd looks ok, sizewise
        if( (bytesReceived < KEV_MSG_HEADER_SIZE) ||
            (bytesReceived != kernEventMsg->total_size)) {
            //ignore
            continue;
        }

        //type cast custom data
        // ->begins right after header
		LOG_MSG *msg = (LOG_MSG*)&kernEventMsg->event_data[0];
		
		uint logStrLen = (uint)strlen(msg->data);
		if (logStrLen==0) continue;
		if (msg->log_type>sizeof(log_types)/sizeof(typeof(log_types))) {
			//printf("unk type: %d\n",msg->log_type);
			continue;
		}
		
		printf("%s: %.*s\n",log_types[msg->log_type],logStrLen,msg->data);
	}
	return 0;
}
