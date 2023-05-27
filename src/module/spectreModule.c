#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>	

#define SECRET_FLAG "Next step: get this module into the mainline kernel"
#define procfs_name "leakTheAddress"
static struct proc_dir_entry *procFile;
static char * secret = NULL;
char temp;

MODULE_DESCRIPTION("Device Driver for exploiting the Spectre V1 vulnerability");
MODULE_VERSION("1");
MODULE_LICENSE("GPL");

//  LDD chapter 2
//      https://static.lwn.net/images/pdf/LDD3/ch02.pdf
//  For a detailed description of the seq_file interface:
//      https://www.kernel.org/doc/html/latest/filesystems/seq_file.html
//  Kernel memory allocation
//      https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html
//      https://static.lwn.net/images/pdf/LDD3/ch08.pdf

static void *ct_seq_start(struct seq_file *s, loff_t *pos){
	if ( *pos == 0 )
		return &secret;
	else
		*pos = 0;
    
    return NULL;
}

static void *ct_seq_next(struct seq_file *s, void *v, loff_t *pos){
    uint8_t i; // for loop declaration only allow in C99 or C11 mode

    *pos = 0;
    // This will read the secret address multiple times
    // and hopefully caching it in order to make the exploit possible
    for(i = 0; i < 10; i++)
        temp = *(secret+1);
    
	return NULL;
}

static void ct_seq_stop(struct seq_file *s, void *v){
    kfree(v);
}

static int ct_seq_show(struct seq_file *s, void *v){
	
    temp  = *(secret+1);
	seq_printf(s, "%16p\n", (void*)secret+1); 

	return 0;
}

static const struct seq_operations ct_seq_ops = {
    .start  = ct_seq_start ,
    .next   = ct_seq_next  ,
    .stop   = ct_seq_stop  ,
    .show   = ct_seq_show  
};

static int ct_open(struct inode *inode, struct file *file){
	return seq_open(file, &ct_seq_ops);
}

static struct file_operations ops = {
    .read    = seq_read,
    .open    = ct_open,
    .owner   = THIS_MODULE,
    .llseek  = seq_lseek,
    .release = seq_release
};

int init_module(void){
    secret = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if(secret == NULL){
        printk(KERN_ALERT "Error: could not allocate memory for secret string\n");
        return -ENOMEM;
    }

    memcpy(secret+1, SECRET_FLAG, 52);

    printk(KERN_ALERT "spectre module initialized\n");
    procFile = proc_create(procfs_name, 0664, NULL, &ops);

    if(procFile == NULL){
        proc_remove(procFile);
        printk(KERN_ALERT "Error: Could not initialize /proc file\n");
        return -ENOMEM;
    }
    return 0 ;
}

void cleanup_module(void){
    proc_remove(procFile);

    kfree(secret);
    secret = NULL;

    printk(KERN_ALERT "spectre module destroyed\n");
}