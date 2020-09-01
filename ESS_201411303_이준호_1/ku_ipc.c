#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>

#include "ku_ipc.h"

#define IOCTL_START_NUM 	0x80
#define IOCTL_NUM1 		IOCTL_START_NUM+1
#define IOCTL_NUM2 		IOCTL_START_NUM+2
#define IOCTL_NUM3 		IOCTL_START_NUM+3

#define IOCTL_EMPTY_Q		0xFFFF0001	

#define IPC_ERROR		0xFFFE0000
#define IPC_UNUSE_Q_ERROR	0xFFFE0001
#define IPC_NOMSG_ERROR		0xFFFE0002
#define IPC_NOVOL_ERROR		0xFFFE0003
#define	IPC_COPY_ERROR		0xFFFE0004
#define IPC_OVERVOL_ERROR	0xFFFE0005
#define IPC_FULL_ERROR		0xFFFE0006


#define DEV_NAME 		"ku_ipc_dev"
#define KUIPC_IOCTL_NUM 	'z'
#define KU_IPC_CHECK		_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IPC_USE_Q		_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_IPC_UNUSE_Q	 	_IOWR(KUIPC_IOCTL_NUM, IOCTL_NUM3, unsigned long *)


MODULE_LICENSE("GPL");
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


struct ku_ipc_msg {
	struct list_head msg_list;	
	struct ku_ipc_buf_info*	buf;
	int len;
};

struct ku_ipc_msg_q {
	struct ku_ipc_msg* msg_head;
	int id;		// 0~9	
	int size;	 
	int vol;
	int use_check;	// if use 1, not use 0
};

///////////////////////
static dev_t dev_num;
static struct cdev *cd_cdev;
spinlock_t ku_ipc_lock;
int msq_q_size;
static struct ku_ipc_msg_q msg_arr[KUIPC_MAXMSG];
//////////////////////////
static struct ku_ipc_msg* ku_ipc_create_msg(struct ku_ipc_buf_info* user_buf, int len, int init_num);
static int ku_ipc_delete_msg(struct ku_ipc_msg* tmp);
static int ku_ipc_q_init(void);
static int ku_ipc_q_reset(int key);
static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int ku_ipc_read(struct file *file, char *buf, size_t len, loff_t* lof);
static int ku_ipc_write(struct file *file, const char *buf, size_t len, loff_t* lof);
static int ku_ipc_release(struct inode *inode, struct file *file);
static int ku_ipc_open(struct inode *inode, struct file *file);
/////////////////////////

static struct ku_ipc_msg* ku_ipc_create_msg(struct ku_ipc_buf_info* user_buf, int len, int init_num) {
	int ret = -1;
	int size;
	struct ku_ipc_msg* tmp = NULL;
	
	/// make space for struct ku_ipc_msg
	tmp = (struct ku_ipc_msg*)kmalloc(sizeof(struct ku_ipc_msg), GFP_KERNEL);
	tmp->buf = (struct ku_ipc_buf_info*)kmalloc(sizeof(struct ku_ipc_buf_info), GFP_KERNEL);
	tmp->buf->msgp = (struct msgbuf*)kmalloc(sizeof(struct msgbuf), GFP_KERNEL);	
	
	
	/// input value
	size = user_buf->msgsz;

	tmp->len = len;
	if(init_num){
		/*
		tmp->buf->msqid = 0;	
		tmp->buf->msgsz = 0;
		tmp->buf->msgflg = 0;
		tmp->buf->msgp->type = 0;
		memset(tmp->buf->msgp->text, '\0', sizeof(char)*128);
		*/
		return tmp;
	}
	tmp->buf->msqid = user_buf->msqid;	
	tmp->buf->msgsz = size;
	tmp->buf->msgflg = user_buf->msgflg;
	tmp->buf->msgp->type = user_buf->msgp->type;
	ret = copy_from_user(tmp->buf->msgp->text, user_buf->msgp->text, size);
	if(ret < 0){
		ku_ipc_delete_msg(tmp);
		return NULL;
	}
	
	return tmp;
}

static int ku_ipc_delete_msg(struct ku_ipc_msg* tmp){
	kfree(tmp->buf->msgp);
	kfree(tmp->buf);
	kfree(tmp);
	return 0;
}

static int ku_ipc_q_init(void){
	struct ku_ipc_msg* tmp = NULL;
	int i;
	for(i = 0; i<10; i++){
		tmp = ku_ipc_create_msg(i, NULL, 0, 1) ;
		INIT_LIST_HEAD(&tmp->msg_list);
		msg_arr[i].msg_head = tmp;		
		msg_arr[i].id = i;
		msg_arr[i].size = 0;
		msg_arr[i].vol = 0;
		msg_arr[i].use_check = 0;
	}
	return 0;
}

static int ku_ipc_q_reset(int key){
	struct ku_ipc_msg* tmp = msg_arr[key].msg_head;	
	struct list_head* next = NULL;
	struct list_head* pos = NULL;	
	list_for_each_safe(pos, next, &tmp->msg_list) {
		ku_ipc_delete_msg(list_entry(pos, struct ku_ipc_msg, msg_list));
	}

	msg_arr[key].use_check = 0;
	return 0;
}


static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {	
	int key = *(int*)arg;
	int ret = -1;
	
	switch(cmd) {
		case KU_IPC_CHECK:
			if(!(msg_arr[key].use_check)){
				return IOCTL_EMPTY_Q;
			}
			break;
		case KU_IPC_USE_Q:			
			msg_arr[key].use_check = 1;
			ret = key;
			break;
		case KU_IPC_UNUSE_Q:
			msg_arr[key].use_check = 0;
			ret = ku_ipc_q_reset(key);			 
			break;
		default:
			break;
	}
	return ret;
}

static int ku_ipc_read(struct file *file, char *buf, size_t len, loff_t* lof){
	struct ku_ipc_msg* tmp_msg = NULL;
	struct ku_ipc_msg* head_msg = NULL;
	struct ku_ipc_msg* tmp = NULL;	
	struct ku_ipc_buf_info* kernel_buf = NULL;
	struct ku_ipc_buf_info* user_buf = NULL;	//	
	struct list_head* next = NULL;
	struct list_head* pos = NULL;
	
	int tmp_id = 0;
	int ret = 0;
	int check = 0;		// check error
	
	spin_lock(&ku_ipc_lock);
	user_buf = (struct ku_ipc_buf_info*)buf;
	tmp_id = user_buf->msqid;
	
	if(!msg_arr[tmp_id].use_check) check = 1;
	if(msg_arr[tmp_id].size < 1) check = 2;
	if(msg_arr[tmp_id].vol < 1) check = 3;
	switch(check){
		spin_unlock(&ku_ipc_lock);
		case 1:
			return IPC_UNUSE_Q_ERROR;
		case 2:
			return IPC_NOMSG_ERROR;
		case 3:
			return IPC_NOVOL_ERROR;
	}
		
	head_msg = msg_arr[tmp_id].msg_head;
	list_for_each_safe(pos, next, &head_msg->msg_list){
		tmp = list_entry(pos, struct ku_ipc_msg, msg_list);
		if(tmp->buf->msgp->type == user_buf->msgp->type) {
			tmp_msg = tmp;
			kernel_buf = tmp_msg->buf;
			break;
		}
	}
	if(!kernel_buf){
		spin_unlock(&ku_ipc_lock);
		return IPC_NOMSG_ERROR;
	}
	if(len >= kernel_buf->msgsz){
		len = kernel_buf->msgsz;
		ret = copy_to_user(user_buf->msgp->text, kernel_buf->msgp->text, len);
		if(ret < 0) {
			spin_unlock(&ku_ipc_lock);
			return IPC_COPY_ERROR;
		}
	} else {
		if((user_buf->msgflg & KU_MSG_NOERROR) != 0) {
			ret = copy_to_user(user_buf->msgp->text, kernel_buf->msgp->text, len);
			if(ret < 0) {
				spin_unlock(&ku_ipc_lock);
				return IPC_COPY_ERROR;
			}
		} else {
			spin_unlock(&ku_ipc_lock);
			return IPC_COPY_ERROR;
		}
	}

	list_del(pos);
	ku_ipc_delete_msg(tmp_msg);
	msg_arr[tmp_id].size --;	
	msg_arr[tmp_id].vol -= kernel_buf->msgsz;	
	spin_unlock(&ku_ipc_lock);
	return len;
}

static int ku_ipc_write(struct file *file, const char *buf, size_t len, loff_t* lof){
	struct ku_ipc_msg* tmp = NULL;
	struct ku_ipc_buf_info* user_buf = (struct ku_ipc_buf_info*)buf;
	
	int ret = 0;
	int check = 0;
	int tmp_id;
	spin_lock(&ku_ipc_lock);
	tmp_id = user_buf->msqid;
	if(!msg_arr[tmp_id].use_check) check = 1;
	if((msg_arr[tmp_id].size + 1) > KUIPC_MAXMSG) check = 2;
	if((msg_arr[tmp_id].vol + len) > KUIPC_MAXVOL) check = 3;
	switch(check){
		spin_unlock(&ku_ipc_lock);
		case 1:
			return IPC_UNUSE_Q_ERROR;
		case 2:
			return IPC_FULL_ERROR;
		case 3:
			return IPC_OVERVOL_ERROR;
	}
	tmp = ku_ipc_create_msg(msg_arr[tmp_id].size + 1, user_buf, len, 1);
	if(!tmp) {
		spin_unlock(&ku_ipc_lock);
		return IPC_ERROR;
	}
	list_add_tail(&tmp->msg_list, &msg_arr[tmp_id].msg_head->msg_list);
	msg_arr[tmp_id].size ++;
	msg_arr[tmp_id].vol += len;
	spin_unlock(&ku_ipc_lock);
	return ret;
}

static int ku_ipc_release(struct inode *inode, struct file *file){
	printk("ku_ipc release\n");
	return 0;
}

static int ku_ipc_open(struct inode *inode, struct file *file){
	printk("ku_ipc open\n");
	return 0;
}


struct file_operations ku_ipc_fops = {
	.unlocked_ioctl = ku_ipc_ioctl,
	.read = ku_ipc_read,
	.write = ku_ipc_write,
	.release = ku_ipc_release,
	.open = ku_ipc_open,

};
static int __init ku_ipc_init(void) {
	int ret;
	printk("ku_ipc init\n");
	msq_q_size = 0;
	ku_ipc_q_init();
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_ipc_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);
	if(ret < 0) {
		printk("device add fail\n");
		return -1;
	}
	
	
	spin_lock_init(&ku_ipc_lock);
	return 0;
}

static void __exit ku_ipc_exit(void) {
	printk("ku_ipc exit\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
