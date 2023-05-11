#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
 
int len,temp;
 
char *msg;
 
ssize_t read_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
    if(count>temp)
    {
        count=temp;
    }
    temp=temp-count;
    copy_to_user(buf,msg, count);
    if(count==0)
        temp=len;
 
    return count;
}
 
ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    copy_from_user(msg,buf,count);
    len=count;
    temp=len;
    return count;
}

static int proc_show(struct seq_file *m, void *v) {
 here:
  seq_printf(m, "proc location: 0x%lx\n", (unsigned long)&&here);
  return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_show, NULL);
}

static struct proc_ops my_fops={
    .proc_open = proc_open,
    .proc_release = single_release,
    .proc_read = read_proc,
    .proc_lseek = seq_lseek,
    .proc_write = write_proc
};
 
void create_new_proc_entry(void)  //use of void for no arguments is compulsory now
{
    proc_create("hello",0,NULL, &my_fops);
    msg=kmalloc(10*sizeof(char), GFP_KERNEL);
}
 
 
int proc_init (void) {
    create_new_proc_entry();
    return 0;
}
 
void proc_cleanup(void) {
    remove_proc_entry("hello",NULL);
    kfree(msg);
}
 
MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);