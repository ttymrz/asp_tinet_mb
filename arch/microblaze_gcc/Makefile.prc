#
#  @(#) $Id: Makefile.prc 5 2012-01-14 09:35:55Z tetsuya $
# 

#
#		Makefile のチップ依存部（Microbalze用）
#

#
#  GNU開発環境のターゲットアーキテクチャの定義
#
GCC_TARGET = mb

#
#  プロセッサ依存部ディレクトリ名の定義
#
PRCDIR = $(SRCDIR)/arch/$(PRC)_$(TOOL)

#
#  コンパイルオプション
#
COPTS := $(COPTS) -mxl-gp-opt -N
LDFLAGS := -nostdlib $(LDFLAGS)
LIBS := $(LIBS) -lgcc

#
#  カーネルに関する定義
#
KERNEL_DIR := $(KERNEL_DIR) $(PRCDIR)
KERNEL_ASMOBJS := $(KERNEL_ASMOBJS) prc_support.o
KERNEL_COBJS := $(KERNEL_COBJS) prc_config.o xps_timer.o

#
#  コンフィギュレータ関係の変数の定義
#
CFG_TABS := $(CFG_TABS) --cfg1-def-table $(PRCDIR)/prc_def.csv

#
#  依存関係の定義
#
cfg1_out.c: $(PRCDIR)/prc_def.csv
kernel_cfg.timestamp: $(PRCDIR)/prc.tf
$(OBJFILE): $(PRCDIR)/prc_check.tf
