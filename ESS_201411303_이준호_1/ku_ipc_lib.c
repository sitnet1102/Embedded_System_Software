#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ku_ipc.h"

#define IOCTL_START_NUM 	0x80
#define IOCTL_NUM1 		IOCTL_START_NUM+1
#define IOCTL_NUM2 		IOCTL_START_NUM+2
#define IOCTL_NUM3 		IOCTL_START_NUM+3

#define IOCTL_EMPTY_Q		0xFFFF0001

#define DEV_NAME 		"ku_ipc_dev"
#define DEV_DIR			"/dev/ku_ipc_dev"
#define KUIPC_IOCTL_NUM 	'z'
#define KU_IPC_CHECK		_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IPC_USE_Q		_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_IPC_UNUSE_Q	 	_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
struct msgbuf {
        long 			type;
        char 			text[128];
};

struct ku_ipc_buf_info {
	struct msgbuf*		msgp;	
	int 			msqid;
	int 			msgflg;
	int 			msgsz;	
};


int ku_msgget(int key, int msgflg);
int ku_msgclose(int msqid);
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg);
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg);

int ku_msgget(int key, int msgflg){
	int tmp = -1;
	int ret = 0;
	int dev = open(DEV_DIR, O_RDWR);
	if(dev < 0) return -1;
	tmp = ioctl(dev, KU_IPC_CHECK, &key);
	if((key<10)&&(key>-1)){
		if(msgflg == KU_IPC_CREAT){
			tmp = ioctl(dev, KU_IPC_USE_Q, &key);
			if(tmp == IOCTL_EMPTY_Q)
				ret = tmp;
			
		}
	}
	else if(msgflg == KU_IPC_EXCL){
		ret = -1;
	}
	close(dev);
	return ret;
}

int ku_msgclose(int msqid){
	int tmp = -1;
	int ret = -1;
	int dev = open(DEV_DIR, O_RDWR);
	tmp = ioctl(dev, KU_IPC_CHECK, &msqid);
	if((msqid<10)&&(msqid>-1)){
		if(!tmp){
			ret = 0;
			tmp = ioctl(dev, KU_IPC_UNUSE_Q, &msqid);
		}
	}else {
		ret = -1;
	}

	close(dev);
	return ret;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg){
	int ret;
	struct ku_ipc_buf_info* buf = NULL;
	int dev = open(DEV_DIR, O_RDWR);
	ret = ioctl(dev, KU_IPC_CHECK, &msqid);
	if(ret != IOCTL_EMPTY_Q){
		close(dev);
		return -1;
	}
	
	buf = (struct ku_ipc_buf_info*)malloc(sizeof(struct ku_ipc_buf_info));
	buf->msqid = msqid;
	buf->msgflg = msgflg;
	buf->msgsz = msgsz;
	buf->msgp = msgp;
	
	ret = write(dev, buf, msgsz);
	if(ret != 0){	
		if(msgflg & KU_IPC_NOWAIT != 0){
			return -1;
		}else{
			while(ret != 0){
				ret = write(dev, buf, msgsz);
			}
			free(buf);	
			close(dev);
			if(ret & IPC_ERROR){
				return -1;
			}
		}
	}
	free(buf);
	close(dev);
	return 0;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg){
	int n = 0;
	int ret = -1;
	int check = 0;
	int dev = open(DEV_DIR, O_RDWR);
	struct ku_ipc_buf_info* buf = NULL;
	ret = ioctl(dev, KU_IPC_CHECK, &msqid);	
	if(ret == IPC_EMPTY_Q){
		close(dev);
		return -1;
	}
	
	buf = (struct ku_ipc_buf_info*)malloc(sizeof(struct ku_ipc_buf_info));
	buf->msqid = msqid;
	buf->msgflg = msgflg;
	buf->msgsz = msgsz;
	buf->msgp = msgp;
	buf->msgp->type = msgtype;	
	
	n = read(dev, buf, msgsz);
	if(n < 0) {
		if(msgflg & KU_IPC_NOWAIT != 0){
			check = 1;
			free(buf);
			close(dev);
			return -1;
		}else{
			check = 2;
			while(n < 0){
				// poling for msg
				n = read(dev, buf, msgsz);
			}
			
		}
	
		if(msgflg & KU_MSG_NOERROR != 0){
			n = read(dev, buf, msgsz);
		}else{
			free(buf);
			close(dev);
			
		}
	}
	if(ret & IPC_ERROR){
		free(buf);
		close(dev);
		return -1;
	}
	free(buf);
	close(dev);
	return n;
}






