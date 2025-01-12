#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x14\x00\x00\x00\x60\x73\x77\x14"
	"cdev_init\0\0\0"
	"\x14\x00\x00\x00\xb1\x19\x54\xaa"
	"cdev_add\0\0\0\0"
	"\x18\x00\x00\x00\xe2\x75\x96\x16"
	"class_create\0\0\0\0"
	"\x18\x00\x00\x00\xc3\x9e\x64\x92"
	"device_create\0\0\0"
	"\x14\x00\x00\x00\x05\xb4\xd2\x39"
	"cdev_del\0\0\0\0"
	"\x1c\x00\x00\x00\xc0\xfb\xc3\x6b"
	"__unregister_chrdev\0"
	"\x18\x00\x00\x00\xf5\x76\x81\x28"
	"class_destroy\0\0\0"
	"\x18\x00\x00\x00\x10\x5e\x65\x38"
	"device_destroy\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\xc2\x9c\xc4\x13"
	"_copy_from_user\0"
	"\x1c\x00\x00\x00\x48\x9f\xdb\x88"
	"__check_object_size\0"
	"\x18\x00\x00\x00\xe1\xbe\x10\x6b"
	"_copy_to_user\0\0\0"
	"\x1c\x00\x00\x00\x61\x94\xb6\x1c"
	"__register_chrdev\0\0\0"
	"\x18\x00\x00\x00\xd7\xd3\x75\x6d"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7EABE5B7A62FEB97104FC7D");
