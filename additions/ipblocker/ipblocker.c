#include<linux/kernel.h>
#include<linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>
#include<linux/list.h> 
#include<linux/string.h>
#include<linux/slab.h>

#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>


MODULE_AUTHOR("blrds");
MODULE_DESCRIPTION("Basic ipblocker");
MODULE_LICENSE("GPL");

struct kobject *kobj_ref;

static struct nf_hook_ops nfin;

struct ipList{
    struct list_head list;
    char* ip;
};

struct list_head Head_Node;
struct list_head *ptr;

///sysfs 
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    int s=0;
    ssize_t size=0;
    struct ipList *obj;
    if(list_empty(&Head_Node)){
        size=sprintf(buf, "there is no elements\n");
    }else{
        list_for_each(ptr, &Head_Node){
            obj=list_entry(ptr,struct ipList,list);
            s+=strlen(obj->ip)+1;
        }
        char *entry=kmalloc(s*sizeof(char), GFP_KERNEL);
        list_for_each(ptr, &Head_Node){
            obj=list_entry(ptr,struct ipList,list);
            strcat(entry,obj->ip);
            strcat(entry,"\n");
        }
        size=sprintf(buf, "%s", entry);
    }
    return size;
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count){
    if(buf[0]=='+'){
        int flag=1;
        if(!list_empty(&Head_Node)){
            struct ipList *obj;
            list_for_each(ptr, &Head_Node){
                obj=list_entry(ptr,struct ipList,list);
                if(!strcmp(obj->ip, &buf[1])){
                    flag=0;
                }
            }
        }
        if(flag){
            struct ipList *temp = kmalloc(sizeof(struct ipList),GFP_KERNEL); 
            temp->ip=kmalloc(sizeof(char)*count, GFP_KERNEL);
            strncpy(temp->ip, &buf[1],strlen(buf)-2);
            INIT_LIST_HEAD(&temp->list);
            list_add(&temp->list, &Head_Node);
        }
    }
    if(buf[0]=='-' && (!list_empty(&Head_Node))){
        struct ipList *obj;
        list_for_each(ptr, &Head_Node){
            obj=list_entry(ptr,struct ipList,list);
            if(!strcmp(&buf[1],obj->ip)){
                list_del(&obj->list);
                break;
            }
        }
    }
    return count;
}

struct kobj_attribute entry_attr = __ATTR(new_entry, 0660, sysfs_show, sysfs_store);


static unsigned int hook_func_in (void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
    struct iphdr *ip_header;
    struct sk_buff *sock_buff;
        
    sock_buff = skb;
    
    if (!sock_buff){
        printk (KERN_INFO "Not socket buffer\n");
        return NF_ACCEPT;
    }
    
    ip_header = (struct iphdr *) skb_network_header (sock_buff);
    if (!ip_header){
        printk (KERN_INFO "Not IP packet\n");
        return NF_ACCEPT;
    }
    char source[16];
    snprintf(source, 16, "%pI4", &ip_header->saddr);
    if(!list_empty(&Head_Node)){
        struct ipList *obj;
        list_for_each(ptr, &Head_Node){
            obj=list_entry(ptr,struct ipList,list);
            printk (KERN_INFO "\"%s\"\n",obj->ip);
            if(!strcmp(obj->ip, source)){
                return NF_DROP;
            }
        }
    }
    return NF_ACCEPT;
}


static int __init init_main(void){
    kobj_ref = kobject_create_and_add("entry_sysfs",kernel_kobj);
    if(sysfs_create_file(kobj_ref,&entry_attr.attr)){
        printk(KERN_INFO"Cannot create sysfs file......\n");
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &entry_attr.attr);
    }
    INIT_LIST_HEAD(&Head_Node);
    nfin.hook = hook_func_in;
    nfin.hooknum = NF_INET_PRE_ROUTING;
    nfin.pf = PF_INET;
    nfin.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook (&init_net, &nfin);
    return 0;
}

static void __exit cleanup_main(void){
    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &entry_attr.attr);
    nf_unregister_net_hook (&init_net, &nfin);
}

module_init(init_main);
module_exit(cleanup_main);