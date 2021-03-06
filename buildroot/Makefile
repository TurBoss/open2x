# Makefile for buildroot2
#
# Copyright (C) 1999-2005 by Erik Andersen <andersen@codepoet.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#--------------------------------------------------------------
# Just run 'make menuconfig', configure stuff, then run 'make'.
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------
TOPDIR=./
CONFIG_CONFIG_IN = Config.in
CONFIG_DEFCONFIG = .defconfig
CONFIG = package/config

noconfig_targets := menuconfig config oldconfig randconfig \
	defconfig allyesconfig allnoconfig release tags    \
	source-check

#	$(shell find . -name *_defconfig |sed 's/.*\///')

# Pull in the user's configuration file
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(TOPDIR).config
endif

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE = $(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet=
  Q =
else
  quiet=quiet_
  Q = @
endif

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)

export CONFIG_SHELL quiet Q KBUILD_VERBOSE

ifndef HOSTAR
HOSTAR:=ar
endif
ifndef HOSTAS
HOSTAS:=as
endif
ifndef HOSTCC
HOSTCC:=gcc
else
endif
ifndef HOSTCXX
HOSTCXX:=g++
endif
ifndef HOSTLD
HOSTLD:=ld
endif
HOSTAR:=$(shell $(CONFIG_SHELL) which $(HOSTAR) || type -p $(HOSTAR) || echo ar)
HOSTAS:=$(shell $(CONFIG_SHELL) which $(HOSTAS) || type -p $(HOSTAS) || echo as)
HOSTCC:=$(shell $(CONFIG_SHELL) which $(HOSTCC) || type -p $(HOSTCC) || echo gcc)
HOSTCXX:=$(shell $(CONFIG_SHELL) which $(HOSTCXX) || type -p $(HOSTCXX) || echo g++)
HOSTLD:=$(shell $(CONFIG_SHELL) which $(HOSTLD) || type -p $(HOSTLD) || echo ld)
ifndef CFLAGS_FOR_BUILD
CFLAGS_FOR_BUILD:="-g -O2"
endif
export HOSTAR HOSTAS HOSTCC HOSTCXX HOSTLD


ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)

# cc-option
# Usage: cflags-y += $(call cc-option, -march=winchip-c6, -march=i586)
# sets -march=winchip-c6 if supported else falls back to -march=i586
# without checking the latter.
cc-option = $(shell if $(TARGET_CC) $(TARGET_CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
	> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#############################################################
#
# Hide troublesome environment variables from sub processes
#
#############################################################
unexport CROSS_COMPILE
unexport ARCH

#############################################################
#
# Setup the proper filename extensions for the host
#
##############################################################
ifneq ($(findstring linux,$(BR2_GNU_BUILD_SUFFIX)),)
HOST_EXEEXT:=
HOST_LIBEXT:=.a
HOST_SHREXT:=.so
endif
ifneq ($(findstring apple,$(BR2_GNU_BUILD_SUFFIX)),)
HOST_EXEEXT:=
HOST_LIBEXT:=.a
HOST_SHREXT:=.dylib
endif
ifneq ($(findstring cygwin,$(BR2_GNU_BUILD_SUFFIX)),)
HOST_EXEEXT:=.exe
HOST_LIBEXT:=.lib
HOST_SHREXT:=.dll
endif
ifneq ($(findstring mingw,$(BR2_GNU_BUILD_SUFFIX)),)
HOST_EXEEXT:=.exe
HOST_LIBEXT:=.lib
HOST_SHREXT:=.dll
endif

# The preferred type of libs we build for the target
ifeq ($(BR2_PREFER_STATIC_LIB),y)
LIBTGTEXT=.a
#PREFERRED_LIB_FLAGS:=--disable-shared --enable-static
else
LIBTGTEXT=.so
#PREFERRED_LIB_FLAGS:=--disable-static --enable-shared
endif
PREFERRED_LIB_FLAGS:=--enable-static --enable-shared


#############################################################
#
# The list of stuff to build for the target toolchain
# along with the packages to build for the target.
#
##############################################################
ifeq ($(BR2_TOOLCHAIN_BUILDROOT),y)
TARGETS:=uclibc-configured binutils gcc uclibc-target-utils
else
TARGETS:=
endif
include toolchain/Makefile.in
include package/Makefile.in

#############################################################
#
# You should probably leave this stuff alone unless you know
# what you are doing.
#
#############################################################

all:   world

# In this section, we need .config
include .config.cmd

# We also need the various per-package makefiles, which also add
# each selected package to TARGETS if that package was selected
# in the .config file.
include toolchain/*/*.mk
include package/*/*.mk

# target stuff is last so it can override anything else
include target/Makefile.in

TARGETS_CLEAN:=$(patsubst %,%-clean,$(TARGETS))
TARGETS_SOURCE:=$(patsubst %,%-source,$(TARGETS))
TARGETS_DIRCLEAN:=$(patsubst %,%-dirclean,$(TARGETS))

world: $(DL_DIR) $(BUILD_DIR) $(STAGING_DIR) $(TARGET_DIR) $(TARGETS)
dirs: $(DL_DIR) $(BUILD_DIR) $(STAGING_DIR)

.PHONY: all world dirs clean dirclean distclean source $(TARGETS) \
	$(TARGETS_CLEAN) $(TARGETS_DIRCLEAN) $(TARGETS_SOURCE) \
	$(DL_DIR) $(BUILD_DIR) $(TOOL_BUILD_DIR) $(STAGING_DIR)

#############################################################
#
# staging and target directories do NOT list these as
# dependencies anywhere else
#
#############################################################
$(DL_DIR) $(BUILD_DIR) $(TOOL_BUILD_DIR):
	@mkdir -p $@

$(STAGING_DIR):
	@mkdir -p $(STAGING_DIR)/bin
	@mkdir -p $(STAGING_DIR)/lib
ifeq ($(BR2_TOOLCHAIN_SYSROOT),y)
	@mkdir -p $(STAGING_DIR)/usr/lib
else
	@ln -snf . $(STAGING_DIR)/usr
	@mkdir -p $(STAGING_DIR)/usr/$(REAL_GNU_TARGET_NAME)
	@ln -snf ../lib $(STAGING_DIR)/usr/lib
	@ln -snf ../lib $(STAGING_DIR)/usr/$(REAL_GNU_TARGET_NAME)/lib
endif
	@mkdir -p $(STAGING_DIR)/usr/include

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)
	if [ -d "$(TARGET_SKELETON)" ] ; then \
		cp -fa $(TARGET_SKELETON)/* $(TARGET_DIR)/; \
	fi;
	touch $(STAGING_DIR)/.fakeroot.00000
	-find $(TARGET_DIR) -type d -name CVS | xargs rm -rf
	-find $(TARGET_DIR) -type d -name .svn | xargs rm -rf

source: $(TARGETS_SOURCE) $(HOST_SOURCE)

.config.check: dependencies
	$(SED) '/BR2_WGET/s/\"$$/ --spider\"/g' .config
	touch $@

_source-check: .config.check
	$(MAKE) source

#############################################################
#
# Cleanup and misc junk
#
#############################################################
clean: $(TARGETS_CLEAN)
	rm -rf $(STAGING_DIR) $(TARGET_DIR) $(IMAGE)

dirclean: $(TARGETS_DIRCLEAN)
	rm -rf $(STAGING_DIR) $(TARGET_DIR) $(IMAGE)

distclean:
ifeq ($(DL_DIR),$(BASE_DIR)/dl)
	rm -rf $(DL_DIR)
endif
	rm -rf $(BUILD_DIR) $(LINUX_KERNEL) $(IMAGE)
	$(MAKE) -C $(CONFIG) clean

sourceball:
	rm -rf $(BUILD_DIR)
	set -e; \
	cd ..; \
	rm -f buildroot.tar.bz2; \
	tar -cvf buildroot.tar buildroot; \
	bzip2 -9 buildroot.tar; \


else # ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)

all: menuconfig

# configuration
# ---------------------------------------------------------------------------

$(CONFIG)/conf:
	$(MAKE) CC="$(HOSTCC)" CFLAGS=$(CFLAGS_FOR_BUILD) MAKECMDGOALS="$(MAKECMDGOALS)" \
		-C $(CONFIG) conf
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi
$(CONFIG)/mconf:
	$(MAKE) CC="$(HOSTCC)" CFLAGS=$(CFLAGS_FOR_BUILD) MAKECMDGOALS="$(MAKECMDGOALS)" \
		-C $(CONFIG) conf mconf
	-@if [ ! -f .config ] ; then \
		cp $(CONFIG_DEFCONFIG) .config; \
	fi

menuconfig: $(CONFIG)/mconf
	@$(CONFIG)/mconf $(CONFIG_CONFIG_IN)

config: $(CONFIG)/conf
	@$(CONFIG)/conf $(CONFIG_CONFIG_IN)

oldconfig: $(CONFIG)/conf
	@$(CONFIG)/conf -o $(CONFIG_CONFIG_IN)

randconfig: $(CONFIG)/conf
	@$(CONFIG)/conf -r $(CONFIG_CONFIG_IN)

allyesconfig: $(CONFIG)/conf
	cp $(CONFIG_DEFCONFIG) .config
	@$(CONFIG)/conf -y $(CONFIG_CONFIG_IN)
	#sed -i -e "s/^CONFIG_DEBUG.*/# CONFIG_DEBUG is not set/" .config

allnoconfig: $(CONFIG)/conf
	@$(CONFIG)/conf -n $(CONFIG_CONFIG_IN)

defconfig: $(CONFIG)/conf
	@$(CONFIG)/conf -d $(CONFIG_CONFIG_IN)

# check if download URLs are outdated 
source-check: allyesconfig
	$(MAKE) _source-check

#############################################################
#
# Cleanup and misc junk
#
#############################################################
clean:
	rm -f .config .config.old .config.cmd .tmpconfig.h
	- $(MAKE) -C $(CONFIG) clean

distclean: clean
	rm -rf sources/*

endif # ifeq ($(strip $(BR2_HAVE_DOT_CONFIG)),y)

%_defconfig: $(CONFIG)/conf
	cp $(shell find ./target/ -name $@) .config
	@$(CONFIG)/conf -o $(CONFIG_CONFIG_IN)


.PHONY: dummy subdirs release distclean clean config oldconfig \
	menuconfig tags check test depend defconfig


