### Lab3

#### Linux的动态模块

##### Makefile文件

```makefile
CONFIG_MODULE_SIG=n
ifneq ($(KERNELRELEASE),)
obj-m := modify_syscall.o       #obj-m指编译成外部模块
else
KERNELDIR := /lib/modules/$(shell uname -r)/build  #定义一个变量，指向内核目录
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules  #编译内核模块
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
all : modify_old_syscall modify_new_syscall
.PHONY : all
modify_old_syscall: modify_old_syscall.c
	gcc -o $@ $<
modify_new_syscall: modify_new_syscall.c
	gcc -o $@ $<
endif
```

内核崩溃：未能处理sys_call_table段的写保护，导致加载动态模块时内核崩溃。

取消段的写保护

```c++
static inline void unprotect_memory(void)
{
    if (update_mapping_prot && start_rodata && init_begin) {
        update_mapping_prot(__pa_symbol(start_rodata),
                            (unsigned long)start_rodata,
                            init_begin - start_rodata,
                            PAGE_KERNEL);
    } else {
        pr_err("Failed to unprotect memory: missing symbols\n");
    }
}
```

添加段的写保护

```c++
static inline void protect_memory(void)
{
    if (update_mapping_prot && start_rodata && init_begin) {
        update_mapping_prot(__pa_symbol(start_rodata),
                            (unsigned long)start_rodata,
                            init_begin - start_rodata,
                            PAGE_KERNEL_RO);
    } else {
        pr_err("Failed to protect memory: missing symbols\n");
    }
}
```

动态获取p_sys_call_table的地址

```c++
unsigned long *get_sys_call_table(void)
{
    if (!p_sys_call_table) {
        p_sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    }
    return p_sys_call_table;
}
```

系统调用序号不对

openEuler基于arm架构与x86架构的系统调用略有不同 `https://arm64.syscall.sh/`



加载动态模块



替换系统调用后



替换系统调用之前



#### Linux的设备驱动

##### Makefile文件

```makefile
CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
        obj-m :=globalvar.o
else
        KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
endif
```

主要功能的实现方式

实现群发（一对多）功能的方式

通过再globalvar实例中添加全局变量total_readers，实时维护读者的数量，提供了每个进程都能读到信息的基础。

实现私发功能@ 在私发模式下，只有是相应进程号的进程才会被唤醒。  

此外，为了防止使用发送端口发送给非法进程（没有对应的进程在等待读），如果没有目标进程，我们并不希望私发的消息被写入缓冲区中被其他读端口读取，所以需要引入检测信号是否合法的机制
 使用

```c++
struct reader_node {
    pid_t pid; // 进程的PID
    struct list_head list; // 列表节点
};
```

 结构，动态维护在等待的读端口，如果私发消息@的进程号匹配不到检任何一个读端口，那么这条消息将被丢弃。

设备文件globalvar.c

#### `globalvar_read` 函数

```C++
static ssize_t globalvar_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    while(globalvar.flag == 1&&globalvar.read_count!=0)
    {
        if(down_interruptible(&globalvar.sem)) //P 操作
        {
            return -ERESTARTSYS;
        }
        up(&globalvar.sem); //V 操作        
    }

    if(wait_event_interruptible(globalvar.outq, globalvar.flag!=0&&(globalvar.private_chat_pid==0||(globalvar.private_chat_pid!=0&&current->pid == globalvar.private_chat_pid)))) //不可读时 阻塞读进程
    {
        return -ERESTARTSYS;
    }

    if(down_interruptible(&globalvar.sem)) //P 操作
    {
        return -ERESTARTSYS;
    }

    if(globalvar.rd < globalvar.wr)
        len = min(len,(size_t)(globalvar.wr - globalvar.rd)); //更新读写长度
    else
        len = min(len,(size_t)(globalvar.end - globalvar.rd));
    globalvar.length=len;
    printk("in read process,len is %ld\n",len);
    if(copy_to_user(buf,globalvar.rd,len))
    {
        printk(KERN_ALERT"copy failed\n");
        up(&globalvar.sem);
        return -EFAULT;
    }
    

    if(globalvar.private_chat_pid==0)    
    {  
        
        globalvar.read_count++; // 增加计数器
        printk("count++\nnow reader count is %d\n",globalvar.read_count);
        // 当计数器的值等于读进程的总数时，更新globalvar.rd的位置
        if(globalvar.read_count == globalvar.total_readers)
        {
            printk("in read process,before newing wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer, globalvar.rd-globalvar.buffer);
            globalvar.rd = globalvar.rd + len;
            
            if(globalvar.rd == globalvar.end)
                globalvar.rd = globalvar.buffer; //字符缓冲区循环
            printk("in read process,after newing wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer, globalvar.rd-globalvar.buffer);
            globalvar.read_count = 0; // 重置计数器
            globalvar.flag=0;
        }
    }
    else//指定私聊进程的操作
    {   
        printk("in @ read process,before newing wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer, globalvar.rd-globalvar.buffer);
        globalvar.rd = globalvar.rd + len;
        globalvar.flag=0;
        if(globalvar.rd == globalvar.end)
            globalvar.rd = globalvar.buffer; //字符缓冲区循环
        printk("in @ read process,after newing wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer, globalvar.rd-globalvar.buffer);
        globalvar.private_chat_pid = 0;
    }
    up(&globalvar.sem); //V 操作
    
    return len;
}
```

#### `globalvar_write` 函数

```c++
static ssize_t globalvar_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{   
    char *kbuf = kmalloc(len + 1, GFP_KERNEL); // 分配内核空间的内存
    int i=-1; char first_char;
    char pid_str[10];
    long tmp;
    struct list_head *pos;
    bool found = false;


    if (!kbuf) 
    {
        printk("kmalloc error\n");
        up(&globalvar.sem); //V 操作
        return -ENOMEM;
    }
    
    if (copy_from_user(kbuf, buf, len)) { // 将用户空间的数据复制到内核空间
            printk("copy_from_user error\n");
            kfree(kbuf);
            up(&globalvar.sem); //V 操作
            return -EFAULT;
        }
    kbuf[len] = '\0'; // 确保字符串以null字符结束
    
    if(down_interruptible(&globalvar.sem)) //P 操作
    {
        return -ERESTARTSYS;
    }

    if (get_user(first_char, buf))
     {
        printk("get_user error\n");
        up(&globalvar.sem); //V 操作
        return -EFAULT;
    }
    if(first_char == '@') //检查是否为私聊消息
    {
        pid_t pid;
        printk("kbuf is %s\n",kbuf);
        for (i = 0; i < len-1 && i < (sizeof(pid_str) - 1) && isdigit(kbuf[i+1]); ++i)
        {
            pid_str[i] = kbuf[i+1];
        }
        pid_str[i] = '\0';


        // 转换为整数
        
        if (kstrtol(pid_str, 10, &tmp)!= 0)
        {
            printk("kstrtoint error\n");
            kfree(kbuf);
            up(&globalvar.sem); //V 操作
            return -EFAULT;
        }
        pid=(pid_t)tmp;

        // 检查指定的进程号是否在读进程的列表中
        list_for_each(pos, &globalvar.readers)
        {
            struct reader_node *node = list_entry(pos, struct reader_node, list);
            if(node->pid == pid)
            {
                found = true;
                break;
            }
        }

            if(!found&&pid!=0)
            {   
                // 指定的进程号不在读进程的列表中，忽略这次操作，但是如果为0，则改为群发
                kfree(kbuf);
                printk(KERN_ALERT "Invalid pid: %d\n", pid);
                up(&globalvar.sem); //V 操作
                return -EINVAL;
            }
            // 指定的进程号在读进程的列表中，设置标志
            globalvar.private_chat_pid = pid;
    }
    
    if(globalvar.rd <= globalvar.wr)
        len = min(len-(i+1),(size_t)(globalvar.end - globalvar.wr));
    else
        len = min(len-(i+1),(size_t)(globalvar.rd - globalvar.wr-1));
    printk("in @write len is %ld\n",len);
    /*if(copy_from_user(globalvar.wr,buf,len))
    {
        up(&globalvar.sem); //V 操作
        return -EFAULT;
    }*/
    strcpy(globalvar.wr, kbuf+i+1);
    printk("in write process,before newing,wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer,globalvar.rd-globalvar.buffer);
    globalvar.wr = globalvar.wr + len;
    
    if(globalvar.wr == globalvar.end)
    	globalvar.wr = globalvar.buffer; //循环
    printk("in write process,after newing,wr=%ld,rd=%ld\n",globalvar.wr-globalvar.buffer,globalvar.rd-globalvar.buffer);
    up(&globalvar.sem);//V 操作
    
    globalvar.flag = 1; //条件成立,可以唤醒读进程
    
    wake_up_interruptible(&globalvar.outq); //唤醒读进程
    printk("write send a number!");
    return len;
}
```

用户测试程序

```c++
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include <unistd.h>
#include <signal.h>


int main()
{
    pid_t pid;
    pid=fork();
    if(pid!=0)//父进程，负责写，给别人发送消息
    {
        //printf("this pid is %d\n",getpid());
        int fd,i;
        char msg[100];
        fd= open("/dev/chardev0",O_WRONLY,S_IRUSR|S_IWUSR);
        
        if(fd!=-1)
        {
            while(1)
            {
                printf("I am %d and I want to have a dialogue with the Three Body World!\n",getpid());
                for(i=0;i<101;i++)  //初始化
                    msg[i]='\0';
                printf("Please input the globar:\n");
                fgets(msg, sizeof(msg), stdin);
                msg[strcspn(msg, "\n")] = '\0';
                write(fd,msg,strlen(msg));
                
                if(strcmp(msg,"quit()")==0)
                {
                    close(fd);
                    kill(pid, SIGKILL);
                    break;
                }
            }
        }
        else
        {
            printf("device open failure\n");
        }
        wait(NULL);
        return 0;
    }
    else//子进程，负责读
    {
        int fd,i;
        char msg[101];
        fd= open("/dev/chardev0",O_RDONLY,S_IRUSR|S_IWUSR);
    
        if(fd!=-1)
        {
            while(1)
            {
                
                for(i=0;i<101;i++)  //初始化
                    msg[i]='\0';
            
                read(fd,msg,100);
                printf("id is %d> ",getpid());
                printf("%s\n",msg);
            
                if(strcmp(msg,"quit()")==0)
                {
                    close(fd);
                    break;
                }
            }
        }
        else
        {
            printf("device open failure,%d\n",fd);
        }
        return 0;
    }
}
```

测试文件为了模仿用户之间的通信，采用在一个终端创建子进程，父进程负责写（发送）消息，子进程负责读（接受）消息。为了进一步实现父进程对子进程的控制，以避免用户退出父进程后子进程陷入僵尸进程，当用户输入quit()时，父进程向子进程发送结束信号，等待子进程结束后再结束自身，避免了僵尸进程占用字符设备。

群发情况：



私发情况：



```bash
scripts/sign-file sha256 ./signing_key.priv ./signing_key.x509 /root/Lab3/os_exp/modify_syscall.ko
```

