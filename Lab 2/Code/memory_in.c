#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/kernel.h>


#define PROCESS_NAME_LEN 32 /*进程名长度*/
#define MIN_SLICE 10        /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0   /*起始位置*/
/*内存分配算法*/
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3
int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;        /*当前分配算法*/
static int pid = 0;              /*初始 pid*/
int flag = 0;                    /*设置内存大小标志*/
int Allocated_Memory_Size = 0;
int Request_Memory_Size = 0;

/*
const MONKEY_FACE = "            __,__
   .--.  .-"     "-.  .--.   \n
  / .. \/  .-. .-.  \/ .. \  \n
 | |  '|  /   Y   \  |'  | | \n
 | \   \  \ 0 | 0 /  /   / | \n
  \ '- ,\.-"""""""-./, -' /  \n
   ''-' /_   ^ ^   _\ '-''   \n
       |  \._   _./  |       \n
       \   \ '~' /   /       \n
        '._ '-=-' _.'        \n
           '-----'           \n
";*/


/*描述每一个；空闲块的数据结构*/
struct free_block_type
{
    int size;
    int start_addr;
    struct free_block_type *next;
};

/*指向内存中空闲块链表的首指针*/
struct free_block_type *free_block;

/*每个进程分配到的内存块的描述*/
struct allocated_block {
    int pid;    int size;   int r_size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
};
/*进程分配内存块链表的首指针*/
struct allocated_block *allocated_block_head = NULL;

// 声明函数
struct free_block_type* init_free_block(int mem_size);
void display_menu();
int set_mem_size();
void set_algorithm();
int new_process();
struct allocated_block* find_process(int pid);
void kill_process();
int display_mem_usage();
int free_mem(struct allocated_block *ab);
int allocate_mem(struct allocated_block *ab);
int dispose(struct allocated_block *free_ab);
void rearrange(int algorithm);

int main(){
    char choice;	pid=0;
    int i = 0;
    free_block = init_free_block(mem_size); //初始化空闲区 
    display_menu();     //显示菜单
    printf(">");
    while(1) {
        //display_menu();     //显示菜单 
        fflush(stdin);
        printf(">");
        choice=getchar();	//获取用户输入 
        printf("\n");
        switch(choice){
            case '1': set_mem_size(); break; //设置内存大小 
            case '2': set_algorithm();flag=1; break;//设置算法 
            case '3': new_process(); flag=1; break;//创建新进程 
            case '4': kill_process(); flag=1;	break;//删除进程
            case '5': display_mem_usage();flag=1; break; //显示内存使用 
            case '0': /*do_exit();*/ exit(0); //释放链表并退出
            default: break;
        }
        	
    }
}

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/ 
struct free_block_type* init_free_block(int mem_size){
    struct free_block_type *fb;
    fb=(struct free_block_type *)malloc(sizeof(struct free_block_type)); 
    if(fb==NULL){
        printf("No mem\n"); 
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;

    fb->next = NULL; 
    return fb;
}

/*显示菜单*/ 
void display_menu(){
    printf("\n");
    printf("---------------  Memory  -------------\n");
    printf("        .--.  .-\"     \"-.  .--.   \n");
    printf("       / .. \\/  .-. .-.  \\/ .. \\  \n");
    printf("      | |  \'|  /   Y   \\  |\'  | | \n");
    printf("      | \\   \\  \\ 0 | 0 /  /   / | \n");
    printf("       \\ '- ,\\.-\"\"\"\"\"\"\"-./, -' /  \n");
    printf("        ''-' /_   ^ ^   _\\ '-''   \n");
    printf("            |  \\._   _./  |       \n");
    printf("            \\   \\ '~' /   /       \n");
    printf("             '._ '-=-' _.'        \n");
    printf("                '-----'           \n");
    printf("--------------------------------------\n");
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE); 
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n"); 
    printf("4 - Terminate a process \n"); 
    printf("5 - Display memory usage \n"); 
    printf("0 - Exit\n");
    printf("--------------------------------------\n");
}

/*设置内存的大小*/ 
int set_mem_size(){
    int size;
    if(flag!=0){ //防止重复设置
        printf("Cannot set memory size again\n"); 
        return 0;
    }
    printf("Total memory size = "); 
    scanf("%d", &size); 
    printf("\n");
    if(size>0) {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag=1; 
    return 1;
}

/* 设置当前的分配算法 */ 
void set_algorithm(){
    int algorithm;
    printf("\t1 - First Fit\n"); 
    printf("\t2 - Best Fit \n"); 
    printf("\t3 - Worst Fit \n"); 
    printf(">> ");
    scanf("%d", &algorithm); 
    printf("\n");
    if(algorithm>=1 && algorithm <=3)
        ma_algorithm=algorithm;
    //按指定算法重新排列空闲区链表 
    rearrange(ma_algorithm);
}

/*按 FF 算法重新整理内存空闲块链表*/ 
void rearrange_FF(){
    if(free_block == NULL) {
        return;
    }

    struct free_block_type *sorted_head = NULL;
    struct free_block_type *current = free_block;
    struct free_block_type *prev_sorted = NULL;
    struct free_block_type *next_sorted = NULL;

    while (current!= NULL) {
        // 取出当前节点
        struct free_block_type *node_to_insert = current;
        current = current->next;

        // 插入到已排序链表的合适位置（按照空闲块大小从小到大排序）
        if (sorted_head == NULL) {
            sorted_head = node_to_insert;
            sorted_head->next = NULL;
        } else {
            prev_sorted = NULL;
            next_sorted = sorted_head;

            while (next_sorted!= NULL && node_to_insert->start_addr > next_sorted->start_addr) {
                prev_sorted = next_sorted;
                next_sorted = next_sorted->next;
            }

            if (prev_sorted == NULL) {
                node_to_insert->next = sorted_head;
                sorted_head = node_to_insert;
            } else {
                node_to_insert->next = next_sorted;
                prev_sorted->next = node_to_insert;
            }
        }
    }

    free_block = sorted_head;
}
/*按 BF 算法重新整理内存空闲块链表*/ 
void rearrange_BF(){
    if(free_block == NULL) {
        return;
    }
    struct free_block_type *sorted_head = NULL;
    struct free_block_type *current = free_block;
    struct free_block_type *prev_sorted = NULL;
    struct free_block_type *next_sorted = NULL;

    while (current!= NULL) {
        // 取出当前节点
        struct free_block_type *node_to_insert = current;
        current = current->next;

        // 插入到已排序链表的合适位置（按照空闲块大小从小到大排序）
        if (sorted_head == NULL) {
            sorted_head = node_to_insert;
            sorted_head->next = NULL;
        } else {
            prev_sorted = NULL;
            next_sorted = sorted_head;

            while (next_sorted!= NULL && node_to_insert->size > next_sorted->size) {
                prev_sorted = next_sorted;
                next_sorted = next_sorted->next;
            }

            if (prev_sorted == NULL) {
                node_to_insert->next = sorted_head;
                sorted_head = node_to_insert;
            } else {
                node_to_insert->next = next_sorted;
                prev_sorted->next = node_to_insert;
            }
        }
    }

    free_block = sorted_head;
}
/*按 WF 算法重新整理内存空闲块链表*/ 
void rearrange_WF(){
    if(free_block == NULL) {
        return;
    }

    struct free_block_type *sorted_head = NULL;
    struct free_block_type *current = free_block;
    struct free_block_type *prev_sorted = NULL;
    struct free_block_type *next_sorted = NULL;

    while (current!= NULL) {
        // 取出当前节点
        struct free_block_type *node_to_insert = current;
        current = current->next;

        // 插入到已排序链表的合适位置（按照空闲块大小从小到大排序）
        if (sorted_head == NULL) {
            sorted_head = node_to_insert;
            sorted_head->next = NULL;
        } else {
            prev_sorted = NULL;
            next_sorted = sorted_head;

            while (next_sorted!= NULL && node_to_insert->size < next_sorted->size) {
                prev_sorted = next_sorted;
                next_sorted = next_sorted->next;
            }

            if (prev_sorted == NULL) {
                node_to_insert->next = sorted_head;
                sorted_head = node_to_insert;
            } else {
                node_to_insert->next = next_sorted;
                prev_sorted->next = node_to_insert;
            }
        }
    }

    free_block = sorted_head;

}

/*按指定的算法整理内存空闲块链表*/ 
void rearrange(int algorithm){
    switch(algorithm){
        case MA_FF: rearrange_FF(); break; 
        case MA_BF: rearrange_BF(); break; 
        case MA_WF: rearrange_WF(); break;
    }
}

/*创建新的进程，主要是获取内存的申请数量*/ 
int new_process(){
    struct allocated_block *ab; 
    int size;	int ret;
    ab=(struct allocated_block *)malloc(sizeof(struct allocated_block)); 
    if(!ab) exit(-5);
    ab->next = NULL; 
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid); 
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name); scanf("%d", &size); printf("\n");
    if(size>0) {ab->size=size; ab->r_size=size;}
    ret = allocate_mem(ab); /* 从空闲区分配内存，ret==1 表示分配 ok*/
    /*如果此时 allocated_block_head 尚未赋值，则赋值*/ 
    if((ret==1) &&(allocated_block_head == NULL)){
        allocated_block_head=ab;
        Allocated_Memory_Size += ab->size;
        Request_Memory_Size += ab->r_size; 
        return 1;	}
    /*分配成功，将该已分配块的描述插入已分配链表*/ 
    else if (ret==1) {
        ab->next=allocated_block_head; 
        allocated_block_head=ab; 
        Allocated_Memory_Size += ab->size;
        Request_Memory_Size += ab->r_size; 
        return 2;	}
    else if(ret==-1){ /*分配不成功*/ 
        printf("Allocation fail\n"); 
        free(ab);
    return -1;
    }
    return 3;
}

/*分配内存模块*/
int allocate_mem(struct allocated_block *ab){ 
    struct free_block_type *fbt, *pre;
    int request_size=ab->size; 
    fbt = pre = free_block;

    //根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
    // 1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
    // 2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
    // 3. 找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
    // 4. 在成功分配内存后，应保持空闲分区按照相应算法有序
    // 5. 分配成功则返回 1，否则返回-1
    //请自行补充。。。。。
    // 遍历空闲分区链表寻找合适的空闲分区
    while (fbt!= NULL) {
        // 情况1：找到可满足空闲分区且分配后剩余空间足够大，则分割
        if (fbt->size >= request_size && fbt->size - request_size >= MIN_SLICE) {
            ab->start_addr = fbt->start_addr;
            ab->size = request_size;

            // 更新空闲分区的信息
            fbt->start_addr = fbt->start_addr + request_size;
            fbt->size -= request_size;

            // 保持空闲分区按照相应算法有序
            rearrange(ma_algorithm);

            return 1;
        }

        // 情况2：找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
        if (fbt->size >= request_size && fbt->size - request_size < MIN_SLICE) {
            ab->start_addr = fbt->start_addr;
            ab->size = fbt->size;

            // 从空闲链表中移除该空闲分区
            if (pre == fbt) {
                free_block = fbt->next;
            } else {
                pre->next = fbt->next;
            }
            // 释放该空闲分区结构体占用的内存
            free(fbt);

            // 保持空闲分区按照相应算法有序
            rearrange(ma_algorithm);

            return 1;
        }
        pre = fbt;
        fbt = fbt->next;
    }

    // 情况3：找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
    if (!fbt) {
        // 先内存紧缩
        struct free_block_type *work = free_block->next;
        while (work!= NULL) {
            free_block->size += work->size;
            struct free_block_type *temp = work;
            work = work->next;
            free(temp);
        }
        free_block->next = NULL;

        struct allocated_block *pre_abt, *abt;
        pre_abt = abt = allocated_block_head;
        while(abt != NULL) {
            if(abt == allocated_block_head) {
                abt->start_addr = DEFAULT_MEM_START;
            } else {
                abt->start_addr = pre_abt->start_addr + pre_abt->size;
            }

            pre_abt = abt;
            abt = abt->next; 
        }

        free_block->start_addr = pre_abt->start_addr + pre_abt->size;

        // 重新遍历合并后的空闲分区链表寻找合适的空闲分区
        pre = fbt = free_block;
        while (fbt!= NULL) {
            // 情况1：找到可满足空闲分区且分配后剩余空间足够大，则分割
            if (fbt->size >= request_size && fbt->size - request_size >= MIN_SLICE) {
                ab->start_addr = fbt->start_addr;
                ab->size = request_size;

                // 更新空闲分区的信息
                fbt->start_addr = fbt->start_addr + request_size;
                fbt->size -= request_size;

                // 保持空闲分区按照相应算法有序
                rearrange(ma_algorithm);

                return 1;
            }

            // 情况2：找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
            if (fbt->size >= request_size && fbt->size - request_size < MIN_SLICE) {
                ab->start_addr = fbt->start_addr;
                ab->size = fbt->size;

                // 从空闲链表中移除该空闲分区
                if (pre == fbt) {
                    free_block = fbt->next;
                } else {
                    pre->next = fbt->next;
                }
                // 释放该空闲分区结构体占用的内存
                free(fbt);

                // 保持空闲分区按照相应算法有序
                rearrange(ma_algorithm);

                return 1;
            }
            pre = fbt;
            fbt = fbt->next;
        }
    }

    return -1;
}

struct allocated_block* find_process(int pid)
{
    struct allocated_block *ab = allocated_block_head;
    while(ab != NULL) {
        if(ab->pid == pid) {
            break;
        }
        ab = ab->next;
    }
    return ab;
}

/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/ 
void kill_process(){
    struct allocated_block *ab; 
    int pid;
    printf("Kill Process, pid="); 
    scanf("%d", &pid); 
    printf("\n");
    ab=find_process(pid); 
    if(ab!=NULL){
        Allocated_Memory_Size -= ab->size;
        Request_Memory_Size -= ab->r_size;
        free_mem(ab); /*释放 ab 所表示的分配区*/ 
        dispose(ab); /*释放 ab 数据结构节点*/
    }
}

/*将 ab 所表示的已分配区归还，并进行可能的合并*/ 
int free_mem(struct allocated_block *ab){
    int algorithm = ma_algorithm;
    struct free_block_type *fbt, *pre, *work;
    fbt=(struct free_block_type*) malloc(sizeof(struct free_block_type)); 
    if(!fbt) return -1;
    // 进行可能的合并，基本策略如下
    fbt->size = ab->size;
    fbt->start_addr = ab->start_addr;
    fbt->next = NULL;
    pre = free_block;
    // 1. 将新释放的结点插入到空闲分区队列末尾
    if(free_block == NULL) {
        free_block = fbt;
    } else {
        while(pre->next != NULL) {
            pre = pre->next;
        }
        pre->next = fbt;
    }
    // 2. 对空闲链表按照地址有序排列
    struct free_block_type *sorted_head = NULL;
    struct free_block_type *current = free_block;
    struct free_block_type *prev_sorted = NULL;
    struct free_block_type *next_sorted = NULL;

    while (current!= NULL) {
        // 取出当前节点
        struct free_block_type *node_to_insert = current;
        current = current->next;

        // 插入到已排序链表的合适位置（按照空闲块大小从小到大排序）
        if (sorted_head == NULL) {
            sorted_head = node_to_insert;
            sorted_head->next = NULL;
        } else {
            prev_sorted = NULL;
            next_sorted = sorted_head;

            while (next_sorted!= NULL && node_to_insert->start_addr > next_sorted->start_addr) {
                prev_sorted = next_sorted;
                next_sorted = next_sorted->next;
            }

            if (prev_sorted == NULL) {
                node_to_insert->next = sorted_head;
                sorted_head = node_to_insert;
            } else {
                node_to_insert->next = next_sorted;
                prev_sorted->next = node_to_insert;
            }
        }
    }

    free_block = sorted_head;
    // 3. 检查并合并相邻的空闲分区
    work = free_block;
    while (work!= NULL && work->next!= NULL) {
        if (work->next->start_addr == work->start_addr + work->size) {
            // 后一个空闲分区与当前空闲分区相邻，进行合并
            work->size += work->next->size;
            // 跳过要合并的下一个空闲分区
            struct free_block_type *t = work->next;
            work->next = work->next->next;
            free(t);
        } else {
            work = work->next;
        }
    }
    // 4. 将空闲链表重新按照当前算法排序
    rearrange(ma_algorithm);
    return 1;
}

/*释放 ab 数据结构节点*/
int dispose(struct allocated_block *free_ab){ 
    struct allocated_block *pre, *ab;
    if(free_ab == allocated_block_head) { /*如果要释放第一个节点*/ 
        allocated_block_head = allocated_block_head->next;
        free(free_ab); 
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next; 
    while(ab!=free_ab){ pre = ab; ab = ab->next; } 
    pre->next = ab->next;
    free(ab);

    return 2;
}


/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */ 
int display_mem_usage(){
    struct free_block_type *fbt=free_block;
    struct allocated_block *ab=allocated_block_head; 
    if(fbt==NULL){ 
        printf("Warning: run out of memory!\n");
        return(-1);
    }
    printf("------------------ Free Memory --------------------\n");


    /* 显示空闲区 */ 
    printf("Free Memory:\n");
    printf("%20s %20s\n", "	start_addr", "	size"); 
    while(fbt!=NULL){
        printf("%20d  %30d\n", fbt->start_addr, fbt->size); 
        fbt=fbt->next;
    }
    printf("---------------------------------------------------\n");
    
    /* 显示已分配区 */ 
    printf("------------------ Used Memory --------------------\n");
    printf("\nUsed Memory: %10d %10d\%\n", Allocated_Memory_Size, Allocated_Memory_Size * 100 / mem_size);
    printf("Fragment: %10d\%\n", 100 * (Allocated_Memory_Size - Request_Memory_Size) / Allocated_Memory_Size);
    printf(" %s %25s %20s %10s\n", "PID", "ProcessName", "start_addr", " size"); 
    while(ab!=NULL){
        printf(" %2d	%21s	%12d	%14d\n",	ab->pid,	ab->process_name, ab->start_addr, ab->size);
        ab=ab->next;
    }

    printf("---------------------------------------------------\n");
    return 0;
}