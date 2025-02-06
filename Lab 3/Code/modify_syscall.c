#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/pgtable.h>

#define sys_No 169  // The syscall number to be hooked (e.g., gettimeofday)

// Function pointer for the original syscall
unsigned long old_sys_call_func;

// Pointer to the syscall table
unsigned long *sys_call_table = NULL;

// Function pointer to update_mapping_prot
void (*update_mapping_prot)(phys_addr_t phys, unsigned long virt, phys_addr_t size, pgprot_t prot);

// Start and end of the .rodata section
unsigned long start_rodata;
unsigned long init_begin;

asmlinkage int hello(const struct pt_regs *regs) // new function
{
    int a = regs->regs[0]; // ��ȡ��һ��������x0
    int b = regs->regs[1]; // ��ȡ�ڶ���������x1
    printk("No 169 syscall has changed to hello\n");
    return a + b; // �������������ĺ�
}



// Dynamically locate the sys_call_table
unsigned long *get_sys_call_table(void)
{
    if (!sys_call_table) {
        sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    }
    return sys_call_table;
}

// Change the memory protection of the .rodata section to read-write
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

// Restore the memory protection of the .rodata section to read-only
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

// Modify the syscall table to replace the target syscall
void modify_syscall(void)
{
    unsigned long *sys_call_addr;

    sys_call_table = get_sys_call_table();
    if (!sys_call_table) {
        pr_err("Failed to find sys_call_table\n");
        return;
    }

    sys_call_addr = &sys_call_table[sys_No];
    old_sys_call_func = sys_call_table[sys_No];  // Save the original syscall

    unprotect_memory();  // Temporarily make .rodata writable
    *sys_call_addr = (unsigned long)&hello;  // Replace the syscall
    protect_memory();  // Restore .rodata to read-only

    pr_info("Syscall 169 hooked successfully!\n");
}

// Restore the original syscall in the syscall table
void restore_syscall(void)
{
    unsigned long *sys_call_addr;

    sys_call_table = get_sys_call_table();
    if (!sys_call_table) {
        pr_err("Failed to find sys_call_table\n");
        return;
    }

    sys_call_addr = &sys_call_table[sys_No];

    unprotect_memory();  // Temporarily make .rodata writable
    *sys_call_addr = old_sys_call_func;  // Restore the original syscall
    protect_memory();  // Restore .rodata to read-only

    pr_info("Syscall 196 restored successfully!\n");
}

// Module initialization
static int __init mymodule_init(void)
{
    pr_info("Loading module...\n");

    // Dynamically locate update_mapping_prot, start_rodata, and init_begin
    update_mapping_prot = (void *)kallsyms_lookup_name("update_mapping_prot");
    start_rodata = (unsigned long)kallsyms_lookup_name("__start_rodata");
    init_begin = (unsigned long)kallsyms_lookup_name("__init_begin");

    if (!update_mapping_prot || !start_rodata || !init_begin) {
        pr_err("Failed to locate required symbols\n");
        return -EINVAL;
    }

    modify_syscall();
    return 0;
}

// Module cleanup
static void __exit mymodule_exit(void)
{
    pr_info("Unloading module...\n");
    restore_syscall();
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_AUTHOR("XJTU-007");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple module named Modify_syscall");
