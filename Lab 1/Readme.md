### Lab1

配置 Vscode-remote连接华为云服务器

**Edit** /etc/ssh/sshd_config

```bash
vi /etc/ssh/sshd_config
```

**Change** AllowTcpForwarding to yes

```
AllowTcpForwarding yes
```

**Restart** sshd

```bash
systemctl restart sshd
```

#### Part 1 Process

**Step 1 :**运行课本 p103 3.7 的程序，结果如下：（对格式稍作调整）

![](C:\Users\H\Desktop\操作系统实验\实验一\images\1.png)

**Step 2 :**删去wait()后，运行结果如下：

![](C:\Users\H\Desktop\操作系统实验\实验一\images\2.png)

可以看出父进程和子进程是并发执行的，父子进程输出顺序随机，在父进程中 `fork()` 的返回值是子进程的pid，而在子进程中 `fork()` 的返回值是0;

去除 `wait(NULL)` 后，子进程运行结束后由于父进程没有执行 `wait()`, 因此其可能变成了僵尸进程(Z+)。

**Step 3 :**

设置全局变量

```c
int value = 0;
```

在父进程中：

```c
value += 2;
```

在子进程中：

```C
value += 1;
```

**运行结果：**

![](C:\Users\H\Desktop\操作系统实验\实验一\images\3.png)

可以看到父进程和子进程中输出的值并不相同，即这两个 `value` 并不是同一个变量。但是它们的地址却一致，这是因为此地址仅为 `value` 在其各自的进程地址空间内的相对地址。而子进程是从父进程 `fork()` 而来的，因此他们的 `value` 的相对地址也一致。

**Step 4 :**

在 return 前增加对全局变量的操作（自行设计）

```c
value += 3;
```

**运行结果：**

![](C:\Users\H\Desktop\操作系统实验\实验一\images\4.png)

可以看到父进程和子进程分别在`return`之前对各自的`value`进行了+3操作。

**Step 5 :**在子进程中调用 system()与 exec 族函数

**运行结果：**

**system()**
****
```c
system("./system_call");
pid1 = getpid();
printf("child: pid = %d ", pid);
printf("child: pid1 = %d\n", pid1);
```
****
![](C:\Users\H\Desktop\操作系统实验\实验一\images\7.png)

****

```c
pid1 = getpid();
printf("child: pid = %d ", pid);
printf("child: pid1 = %d\n", pid1);
system("./system_call");
```
****

![](C:\Users\H\Desktop\操作系统实验\实验一\images\8.png)



**exe函数**
****
```c
execl("./system_call", NULL);
pid1 = getpid();
printf("child: pid = %d ", pid);
printf("child: pid1 = %d\n", pid1);
```
****
![](C:\Users\H\Desktop\操作系统实验\实验一\images\5.png)

****
```c
pid1 = getpid();
printf("child: pid = %d ", pid);
printf("child: pid1 = %d\n", pid1);
execl("./system_call", NULL);
```
****

![](C:\Users\H\Desktop\操作系统实验\实验一\images\6.png)

在子进程中开始处调用 `execl("./system_call",NULL);`， 此处的 `"./system_call"` 的作用是输出进程的 pid 值。可以看到子进行直接去执行了 `./system_call`，不会执行源程序中打印 pid 的部分。这是因为当进程调用`execl`时该进程的用户空间和数据将被新程序替换。

如果我们换成在子进程的末尾处调用，则：则子进程会把原来进程中打印 pid 的部分执行完再去执行 `system_call`, 这两项工作都是在同一个进程(**子进程**)中进行的(两次打印的 pid 值相同)。

使用`system()`调用时， `system(...)` 内实际上调用了一次 `fork()`,也就是子进程创建出来一个自己的子进程，即父进程的"**孙子进程**"去执行 `./system_call` (因为 `./system_call` 中输出的 pid 不同于子进程执行源程序时输出的 pid)。因此无论`system()`在子进程开始处或者末尾处调用，输出均相同。

#### Part2 Thread

编译代码，由于调用`pthread.h`，应在`gcc`末尾添加`-lpthread`

```bash
gcc thread0.c -o thread0 -lpthread
```

**Step 1 :**创建两个子线程，两线程分别对同一个共享变量多次操作，观察输出结果。

创建两个子线程，分别对共享变量进行100000次+/-100操作。func1进行+100操作，func2进行-100操作。

```c
/* create the thread */
pthread_create(&thread1, &attr1, func1, NULL);
pthread_create(&thread2, &attr2, func2, NULL);
```

调用 `pthread_join(tid, NULL)` 对两个线程进行阻塞等待，并在结束后分别打印 `value` 的值。

```c
pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
```

```c
// Thread 1
for(i = 0; i < 100000; i++) {
    value += 100;
}
// Thread 2
for(i = 0; i < 100000; i++) {
    value -= 100;
}
```

**运行结果：**

![](C:\Users\H\Desktop\操作系统实验\实验一\images\9.png)

可以观察到，多次运行程序，输出结果不同。与预期输出结果0并不一致。由于两个线程均要对`value`的值进行修改，但是却没有实现互斥访问，造成结果输出错误。

**Step 2: **定义信号量 signal，使用 PV 操作实现共享变量的访问与互斥。

定义初始化信号量

```C
sem_t mutex; /* create signal */
...
// 互斥
sem_wait(&mutex);
value += 100;
sem_post(&mutex);
```

**运行结果：**

![](C:\Users\H\Desktop\操作系统实验\实验一\images\10.png)

可以观察到，实现了两个线程对共享变量的互斥访问，输出正确结果0。

**Step 3: **system()与 exec 族函数的基础上，将这两个函数的调用改为在线程中实现，输出进程 PID 。使用`syscall(SYS_gettid)`线程的 TID （`syscall(SYS_gettid)`是通过内核分配的实际线程 tid，在系统范围内唯一的）进行分析。

![](C:\Users\H\Desktop\操作系统实验\实验一\images\system.png)

****

![](C:\Users\H\Desktop\操作系统实验\实验一\images\exe.png)

可以观察到，多次运行程序之后，输出结果的顺序有所不同，与Part1相同，`system()` 调用时在线程中创建新进程来运行被调用的外部程序，而`execl()`调用时，直接将外部程序加载进当前进程，替换掉线程的后续执行代码。

#### Part3 Spin Lock

**Step 1:** 补充的主函数代码。

```c
int main() {
    pthread_t thread1, thread2;
    spinlock_t lock;

    // 输出共享变量的值
    printf("Shared value: %d\n", shared_value);

    // 初始化自旋锁
    spinlock_init(&lock);

    int ret;
    // 创建两个线程
    ret = pthread_create(&thread1, NULL, thread_function, &lock);
    if(ret == 0) {
        printf("thread1 create success!\n");
    }

    ret = pthread_create(&thread2, NULL, thread_function, &lock);
    if(ret == 0) {
        printf("thread2 create success!\n");
    }
    
    // 等待线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // 输出共享变量的值
    printf("Shared value: %d\n", shared_value);

    return 0;
}
```

**运行结果：**

![](C:\Users\H\Desktop\操作系统实验\实验一\images\11.png)

可以观察到自旋锁用于在线程竞争共享资源的同步机制，输出结果与预期结果一致。使用自旋锁可以保证同一时间只有一个线程在对临界区进行操作，而其他线程处于盲等状态，保证了对临界区的互斥访问。



