#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x6f3cc8d6, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x4cd715f4, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x34083a97, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0x79aa04a2, __VMLINUX_SYMBOL_STR(get_random_bytes) },
	{ 0xb738e26c, __VMLINUX_SYMBOL_STR(thc_init) },
	{ 0x920f13e1, __VMLINUX_SYMBOL_STR(_thc_pendingfree) },
	{ 0x846f59a, __VMLINUX_SYMBOL_STR(cpu_info) },
	{ 0x3ff3952e, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xf4804989, __VMLINUX_SYMBOL_STR(_thc_lazy_awe_marker) },
	{ 0x21942b31, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x8693c29f, __VMLINUX_SYMBOL_STR(set_cpus_allowed_ptr) },
	{ 0x7e192dff, __VMLINUX_SYMBOL_STR(_thc_endasync) },
	{ 0xe221aa2a, __VMLINUX_SYMBOL_STR(_thc_endfinishblock) },
	{ 0xb6029609, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x93fca811, __VMLINUX_SYMBOL_STR(__get_free_pages) },
	{ 0x3c30ace4, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0x58bec661, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x2084cb51, __VMLINUX_SYMBOL_STR(THCYield) },
	{ 0x4302d0eb, __VMLINUX_SYMBOL_STR(free_pages) },
	{ 0xe4a7ae79, __VMLINUX_SYMBOL_STR(thc_latch_init) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x141b2c50, __VMLINUX_SYMBOL_STR(__put_task_struct) },
	{ 0x407cc815, __VMLINUX_SYMBOL_STR(_thc_startfinishblock) },
	{ 0xc6b4b936, __VMLINUX_SYMBOL_STR(thc_done) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "2328F1CEE67854C5E38D6AA");
