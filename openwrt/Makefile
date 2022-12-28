
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=extkmod
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define CHECK_KMOD_CONFIG
  KERNEL_EXTRA_FLAGS += $(if $(CONFIG_PACKAGE_kmod-$(1)),CONFIG_EXTKMOD_$(call toupper,$(1))=m)
  PKG_KMOD_LIST += $(1)
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) $(KERNEL_EXTRA_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

define KernelPackage/extkmod-default
  SECTION:=example
  CATEGORY:=Example
endef

define KernelPackage/hello
$(call KernelPackage/extkmod-default)
  TITLE:=The hello example
  FILES:=$(PKG_BUILD_DIR)/hello/hello.ko
endef

$(eval $(call CHECK_KMOD_CONFIG,hello))

define KernelPackage/world
$(call KernelPackage/extkmod-default)
  TITLE:=The world example
  FILES:=$(PKG_BUILD_DIR)/world/world.ko
endef

$(eval $(call CHECK_KMOD_CONFIG,world))

$(eval $(foreach PKG,$(PKG_KMOD_LIST),$(call KernelPackage,$(PKG))))
