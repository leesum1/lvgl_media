#
# Makefile
#

CROSS_COMPILE = /home/leesum/tina-d1-h/prebuilt/gcc/linux-x86/riscv/toolchain-thead-glibc/riscv64-glibc-gcc-thead_20200702/bin/riscv64-unknown-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CXX =$(CROSS_COMPILE)g++
PROJECT_DIR ?= ${shell pwd}
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= $(PROJECT_DIR)


WARNINGS_CFG ?= -Wall -Wextra \
				-Wshadow -Wundef -Wmaybe-uninitialized \
				-Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
				-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
				-Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
				-Wempty-body -Wshift-negative-value -Wstack-usage=4096 \
				-Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith

# 头文件目录
LIB_DIR = /home/leesum/tina-d1-h/out/d1-h-nezha/staging_dir/target/usr/lib/
LIB_INC = /home/leesum/tina-d1-h/out/d1-h-nezha/staging_dir/target/usr/include/

INCLUDE_CFG += -I$(LVGL_DIR)/  
INCLUDE_CFG += -I$(PROJECT_DIR)/USER/App
INCLUDE_CFG += -I$(PROJECT_DIR)/USER/App/src
INCLUDE_CFG += -I$(PROJECT_DIR)/USER/App/Utils/ArduinoJson/src 
INCLUDE_CFG += -I$(LIB_INC)

# 编译选项设置
CFLAGS ?= -O3   $(INCLUDE_CFG) $(WARNING_CFG)
CXXFLAGS ?= $(CFLAGS)
LDFLAGS ?= -lm -lstdc++ -lts -lpthread  -L$(LIB_DIR)

BIN = demo


#Collect the files to compile
include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

# 添加源文件
CSRCS += $(shell find -L $(PROJECT_DIR)/USER/src -name "*.c")
CSRCS += $(shell find -L $(PROJECT_DIR)/USER/App -name "*.c")
CXXSRCS += $(shell find -L $(PROJECT_DIR)/USER/src -name "*.cpp")
CXXSRCS += $(shell find -L $(LVGL_DIR)/USER/HAL -name "*.cpp")
APP_CXXSRCS = $(shell find -L $(PROJECT_DIR)/USER/App -name "*.cpp")
# main 源文件
MAINSRC = $(PROJECT_DIR)/USER/src/main.cpp

# 去除多余测试main函数
APP_FILTER_OUT += $(shell find  $(PROJECT_DIR)/USER/App/Utils/ArduinoJson -name "*.cpp")
APP_FILTER_OUT += $(shell find  $(PROJECT_DIR)/USER/App/Utils/lv_img_png -name "*.cpp")
CXXSRCS += $(filter-out $(APP_FILTER_OUT), $(APP_CXXSRCS))

# 由源文件生成 obj 文件
OBJEXT ?= .o
AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cpp=$(OBJEXT))
# main 函数
MAINOBJ = $(MAINSRC:.cpp=$(OBJEXT))

#汇总源文件.c 和目标文件.o
SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS = $(MAINOBJ) $(AOBJS) $(COBJS) $(CXXOBJS) 

DEPS = $(MAINSRC:.cpp=.d) $(CSRCS:.c=.d) $(CXXSRCS:.cpp=.d) 
## MAINOBJ -> OBJFILES

all: default
-include $(DEPS)

# 把所有的[.c]文件都编译成[.o]文件，并打印输出
%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@ -MD -MF $*.d -MP
	@echo "CC $<"

# 把所有的[.cpp]文件都编译成[.o]文件，并打印输出
%.o: %.cpp
	@$(CXX)  $(CXXFLAGS) -c $< -o $@ -MD -MF $*.d -MP
	@echo "CXX $<"

default: $(OBJS)
	$(CC) -o $(BIN) $(AOBJS) $(COBJS) $(CXXOBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN)  $(OBJS) $(DEPS)

