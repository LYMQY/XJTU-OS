### Lab2

#### 软中断通信

编写程序，补充完整两个函数

```c++
void inter_handler() {
	flag = 1;
    switch(sig)
	{
		case SIGINT:
			printf("\n2 stop test\n");
			break;
		case SIGQUIT:
			printf("\n3 stop test\n");
			break;
		case SIGSTKFLT:
			printf("\n16 stop test\n");
			break;
		case SIGCHLD:
			printf("\n17 stop test\n");
			break;
		case SIGALRM:
			printf("\n14 stop test\n");
			break;
		default: break;
	}
}

void waiting() {
	while (flag == 0) {
		pause();
	}
}
```

父进程中

```c
signal(SIGQUIT, inter_handler);
signal(SIGINT, inter_handler);
signal(SIGALRM, inter_handler);

// Parent Process
alarm(5);
waiting();
//sleep(5);
			
kill(pid1, 16);
kill(pid2, 17);
			
wait(NULL);
wait(NULL);
printf("\nParent process is killed!!\n");
```

子进程中

```c
// Child Process2
signal(17, inter_handler);
signal(SIGQUIT, SIG_IGN);
signal(SIGINT, SIG_IGN);
signal(SIGALRM, SIG_IGN);
pause();
printf("\nChild process2 is killed by parent!!\n");
return 0;

// Child Process1
signal(16, inter_handler);                                                        
signal(SIGQUIT, SIG_IGN);
signal(SIGINT, SIG_IGN);
signal(SIGALRM, SIG_IGN); 
pause(); 
printf("\nChild process1 is killed by parent!!\n");
return 0;
```

其中，

```C
signal(SIGQUIT, SIG_IGN);
signal(SIGINT, SIG_IGN);
signal(SIGALRM, SIG_IGN);
```

用于在子进程中忽略`SIGQUIT`、`SIGINT`、`SIGALRM`信号。

运行结果：

（1）你最初认为运行结果会怎么样？写出你猜测的结果。

​	如果5s内从键盘手动输出`Ctrl+C`发出中断信号，那么应当首先输出截获`SIGINT`信号对应的输出。接下来两个子进程的输出顺序是随机的，但是要求每个子进程需要满足，先输出捕获信号后执行函数对应的输出，然后打印输出子进程被父进程杀死的语句，最后输出父进程被回收的对应语句。

（2）实际的结果什么样？有什么特点？在接收不同中断前后有什么差别？请将 5 秒内中断

和 5 秒后中断的运行结果截图，试对产生该现象的原因进行分析。

​	**实际结果：**

***5s内中断***



***5s后中断***



​	5s内手动发出信号会使得父进程中多捕获一个软中断信号进而比5s后中断的情形多执行一次inter_handle()函数进而得到第一条额外的输出，而5s后父进程不再休眠等待因而不执行这条额外的输出。

（3）改为闹钟中断后，程序运行的结果是什么样子？与之前有什么不同？

​	改为闹钟中断后，仅会影响5s后中断的结果

***5s后中断***



​	与之前的不同是，这次的实现设置了`alarm()`函数和`waiting()`函数，`alarm`函数运行设置的秒数后会发出 `SIGALRM` 信号给父进程，父进程会捕获一个软中断信号，而这会中断`waiting()`语句，进而实现对`kill()`函数的调用。

（4）kill 命令在程序中使用了几次？每次的作用是什么？执行后的现象是什么？

​	`kill`命令在程序中使用了两次，作用是向子进程发送中断信号结束此进程，第一次执行后，子进程接受16信号并打印`16 stop test`，并打印子进程1被杀死的信息，第二次执行同理，打印`17 stop test`，并打印子进程2被杀死的信息。

（5）使用 kill 命令可以在进程的外部杀死进程。进程怎样能主动退出？这两种退出方式

哪种更好一些？

​	进程主动退出方式：**正常退出**：程序在完成所有任务后，执行系统调用（如在 C 语言中调用`exit()`函数）正常结束进程。这种方式会让程序有机会释放所占用的资源，如关闭文件、释放内存等。exit()允许进程向父进程传递一个退出状态。这个退出状态可以被父进程获取，用于了解子进程的退出情况。`kill` 命令不能提供这种方式。使用kill命令在进程外部强制其退出可能导致进程的数据丢失或资源泄漏，但当进程出现异常无法退出时，可能需要kill命令来中止进程。

---

#### 管道通信

根据所给代码补充完在父(子)进程中读写的部分代码

```C
lockf(fd[1], 1, 0);	    // 锁定管道
for(int i = 0; i < 2000; i++)
{
    write(fd[1], &c1, 1);// 分2000次每次向管道写入字符‘1’
}
sleep(5);	            // 等待读进程
lockf(fd[1], 0, 0);	    // 解除管道的锁定
exit(0);	            // 结束进程1

lockf(fd[1],1,0);
for(int i = 0; i < 2000; i++)
{
    write(fd[1], &c2, 1);   // 分2000次每次向管道写入字符‘2’
}
sleep(5);
lockf(fd[1],0,0);
exit(0);

wait(0); // 等待子进程1结束
wait(0); // 等待子进程2结束
lockf(fd[0], 1, 0);
read(fd[0], InPipe, 4000);         // 从管道中读出4000个字符
InPipe[4000] = '\0';               // 加字符串结束符
lockf(fd[0], 0, 0);
printf("%s\n",InPipe);   	       // 显示读出的数据 
exit(0);
```

在子进程中，删除lockf函数

```C
//lockf(fd[1], 1, 0);
、、、
//lockf(fd[1], 0, 0);
```

则管道读写的互斥和同步出现问题，输出结果如下:



加上之后输出结果正确



(1)你最初认为运行结果会怎么样？

**运行结果**应该会先输出2000个***1***，再输出2000个***2***。

(2)实际的结果什么样？有什么特点？试对产生该现象的原因进行分析。

**分析：**有锁状态下，同一时间内只有一个进程可以写入，因而是先写入了2000个1而后写入了2000个2。

无锁状态下，两进程对InPipe的写入在时序上是随机的，因而打印结果也是1与2交错打印

(3)实验中管道通信是怎样实现同步与互斥的？如果不控制同步与互斥会发生什么后果？

同步与互斥的实现：在对管道写入前，先对管道用`lockf()`函数加锁，等管道写入完毕后，在将管道解锁。

不控制的后果：由于多个进程同时向管道中写入数据，那么数据就很容易发生交错和覆盖，导致数据错误。

----

#### 内存的分配与回收

根据实验要求和所学知识补全代码：

**FF算法**

- **思想：** 分配时从内存的起始位置开始找到第一个足够大的空闲块进行分配。

- **优点：** 简单，实现容易。

- **缺点：** 可能会导致大块的内存碎片。

  ```c
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
  ```

  

**BF算法**

- **思想：** 分配时找到所有足够大的空闲块中最小的一个进行分配。

- **优点：** 尽量减小内存碎片。

- **缺点：** 查找最小空闲块可能比较耗时。

  ```c
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
  ```

  

**WF算法**

- **思想：** 分配时找到所有足够大的空闲块中最大的一个进行分配。

- **优点：** 可以避免产生大量小碎片。

- **缺点：** 可能导致大块的内存碎片。

  ```C
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
  ```

  **分配内存模块**

  ```c
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
  ```

  **释放分配区模块**

  ```c
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
  ```

  测试数据设计

  ```bash
  // test3.txt
  // 设置内存大小
  1
  1024
  // 创建3个进程
  3
  100
  3
  600
  3
  100
  5 // 展示内存分配情况
  // 删除2号进程
  4
  2
  5 // 展示内存分配情况
  3
  200
  5 // 展示内存分配情况
  选择BF算法
  2
  2
  5 // 展示内存分配情况
  创建大小为100的进程
  3
  100
  5 // 展示内存分配情况
  选择FF算法
  2
  1
  5 // 展示内存分配情况
  创建大小为10的进程
  3
  10
  5 // 展示内存分配情况
  选择WF算法
  2
  3
  5 // 展示内存分配情况
  创建大小为30的进程
  3
  30
  5 // 展示内存分配情况
  0 // 退出
  
  ```

  **运行截图**

```bash
./memory < test3.txt
```

**主界面**



**内存碎片**



**内存紧缩**



**BF**



**FF**



**WF**



（2）3 种算法的空闲块排序分别是如何实现的。

​	在代码中，空闲块的排序是通过三个函数实现的：

- **rearrange_FF()：** 对空闲块按照起始地址进行插入排序。
- **rearrange_BF()：** 对空闲块按照大小进行从小到大插入排序。
- **rearrange_WF()：** 对空闲块按照大小进行从大到小逆向插入排序。

​	这些函数在每次内存分配或释放后都被调用，以保持空闲块的有序性。

（3）结合实验，举例说明什么是内碎片、外碎片，紧缩功能解决的是什么碎片。

- **内碎片：** 指已分配给进程的内存块中，没有被进程使用的部分。例如，如果一个进程请求分配100个字节，但系统只能提供102个字节的内存块，那么有2个字节就是内碎片。
- **外碎片：** 指分配给各进程的内存块之间存在的不可用的、无法分配的小块内存。例如，多次的进程分配和释放导致内存中存在很多不连续的小块，这些小块之间的未分配空间就是外碎片。
- **紧缩功能解决的碎片：** 当有大量外碎片时，紧缩功能可以将已分配的内存块整理并移动，以便合并这些碎片，形成更大的连续可用空间。这样可以提高内存的利用率，减少外碎片对系统的影响。

（4）在回收内存时，空闲块合并是如何实现的？

​	在回收内存时，空闲块合并是通过在 `free_mem()` 函数中实现的。具体步骤如下：

1. 将被释放的内存块信息创建为一个新的空闲块节点 `fbt`。
2. 将这个新的空闲块节点的`fbt` 插入当前的空闲块链表的尾部。
3. 对当前空闲块链表进行rearrange_FF()即按照起始地址进行插入排序。
4. 对当前的空闲块链表进行遍历，检查是否有相邻的空闲块可以合并。
5. 如果找到相邻的空闲块，则合并它们，更新相应的信息。
6. 继续遍历直到整个空闲块链表。

​	这样，通过合并相邻的空闲块，可以尽量减小内存碎片，提高内存利用率。

#### 页面置换

FIFO



LRU



FIFO(BLEADY)



**结果**：

1.当需要调入页面到内存时，选择最先调入页面的那个置换掉。

2.当需要调入页面到内存时，选择最近最少使用的页面置换掉。

3.当内存页面数增加了以后，命中率反而降低了。

4.具有局部性良好的数据，LRU命中率高于FIFO算法

**结果解释：**

1.使用FIFO算法，创建一个队列来管理所有的内存页面，置换的是队列的首个页面。当需要调入页面到内存时，就将它加到队列的尾部。

2.LRU 置换将每个页面与它的上次使用的时间关联起来。当需要置换页面时，LRU 选择最长时间没有使用的页面。代码中为每个页表条目关联一个使用时间域clock，并为 CPU 添加一个全局的计数器，每次内存引用都会递增时钟，并将时钟寄存器的内容复制到对应页表条目的使用时间域，这样每次最小的时钟域就是要被置换的页面。

3.Belady 异常指的是， 对于FIFO算法，随着内存分配帧数量的增加，命中率可能会下降。

4.LRU具有良好的局部性，对于最近出现过的页面，更不可能被换出，而局部性数据一般会多次重复访问，所以命中率会更高。
