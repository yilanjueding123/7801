######################################################
#
# Comple tools path assign
#
######################################################

OUTPUT_LIB_NAME = gplib_fs.a

SRC_TOP_PATH= X:
MAKETOOLS_PATH= $(SRC_TOP_PATH)/tools/make_tools

.SUFFIXES:

sinclude	$(MAKETOOLS_PATH)/Makefile.utl


######################################################
#
# ENDIAN select
#
######################################################

ENDIAN          := LITTLE
#ENDIAN         := BIG

ifeq "$(ENDIAN)" "LITTLE"
  ENDIAN        = -EL
  GOAL					= gplib_fs.a
  CFLAGS_ENDIAN = -DLITTLE_ENDIAN
else
  ENDIAN        =
  GOAL					= gplib_fs.a
  CFLAGS_ENDIAN = -DBIG_ENDIAN
endif

######################################################
#
# Set inlcude file path
#
######################################################
INCLUDE_DIR	= -I../ \
              -I$(SRC_TOP_PATH)/driver_l1/common/inc \
              -I$(SRC_TOP_PATH)/gplib/fs/include \
              -I$(SRC_TOP_PATH)/gplib/fs/driver \
						  -I$(SRC_TOP_PATH)/project/$(PROJECTNAME)/inc	\
						  -I$(SRC_TOP_PATH)/gplib/inc \
						  -I$(SRC_TOP_PATH)/gpstdlib/inc \
						  -I$(SRC_TOP_PATH)/driver_l1/inc	\
						  -I$(SRC_TOP_PATH)/driver_l2/inc	\
						  -I$(SRC_TOP_PATH)/os/inc 
						  
						 

######################################################
#
# compile option
#
######################################################

CFLAGS_C        = \
								-O2	\
								-W -Wall \
								$(INCLUDE_DIR) \
								-D__FS_AVOID__ \
								-D$(COMPILER) 
#								-D__GCC_COMPILE__	\
#								-fstrength-reduce	\
#								-fthread-jumps	\
#								-fverbose-asm	\
#								-fmessage-length=0	\
                

CFLAGS_S        =  $(INCLUDE_DIR)

CFLAGS_EXTRA	= 
CFLAGS_COMPILE	=

#CFLAGS_ARM	=  -msoft-float -mcpu=arm7tdmi $(CFLAGS_C) $(CFLAGS_EXTRA) $(CFLAGS_COMPILE)
CFLAGS_ARM	=  $(CFLAGS_C) $(CFLAGS_EXTRA) $(CFLAGS_COMPILE)
CFLAGS_ARM_SS = 


######################################################
#
# source code assign
#
######################################################
	
SOURCES = $(wildcard ../*.c)	\
					$(wildcard ../api/*.c)	\
					$(wildcard ../file_system/*.c)	\
					$(wildcard ../interface/*.c)	\
					$(wildcard ../logic_block/*.c)	\
					$(wildcard ../long_file_name/*.c)	\
					$(wildcard ../unicode/*.c)
#					$(wildcard ../driver/*.c)	\
#					$(wildcard ../nls/nls_ascii.c)	\
#					$(wildcard ../nls/nls_nvram_cp950_cp932_cp936.c)	\
#					$(wildcard ../nls/nls_codepage.c)	
		 
SOURCES_ASM = $(wildcard ../*.s)	\
					$(wildcard ../api/*.s)	\
					$(wildcard ../file_system/*.s)	\
					$(wildcard ../interface/*.s)	\
					$(wildcard ../logic_block/*.s)	\
					$(wildcard ../long_file_name/*.s)	\
					$(wildcard ../unicode/*.s)
					
OBJS		= $(addprefix ./,$(SOURCES:.c=.o)) $(addprefix ./,$(SOURCES_ASM:.s=.o))

ASMS		= $(DEPENDS:.d=.s))
DISASMS		= $(DEPENDS:.d=.dis))


.PHONY:		all clean depends  


all:		$(GOAL)
	@echo "all made"

clean:
	@echo "remove all generated files"
	@call del ..\*.o
	@call del ..\api\*.o
 
	@call del ..\file_system\*.o	 
	@call del ..\interface\*.o	 
	@call del ..\logic_block\*.o	 
	@call del ..\long_file_name\*.o	 			
	@call del ..\unicode\*.o	 		
	@call del .\*.a 
#	@call del ..\driver\*.o		
#	@call del ..\nls\*.o	

depends: $(DEPENDS)
	@echo "dependency made"

$(OUTPUT_LIB_NAME) : $(OBJS)
	$(XAR) -cr $@ $(OBJS)

#ifneq ($(MAKECMDGOALS),clean)
#sinclude	$(DEPENDS)
#endif

#./%.d : %.c
#	@$(PERL) $(MAKETOOLS_PATH)/wecho.pl  \"generating dependency for $<\"
#	@$(MKDIR) -p $(dir $@)
#	@$(XCC) $(CFLAGS_ARM) -M -o $(MAKETOOLS_PATH)/depends.tmp $<
#	@$(PERL) $(MAKETOOLS_PATH)/makedep.pl $(MAKETOOLS_PATH)/depends.tmp $@


./%.o : %.c
	@call $(MAKETOOLS_PATH)/wecho.exe $< cross-compiling gcc\"
	@$(XCC) $(CFLAGS_ARM) -c -o $@ $<

./%.o : %.S
	@call $(MAKETOOLS_PATH)/wecho.exe $< cross-compiling asm\"
	@$(XAS) $(CFLAGS_ARM_S) $(CFLAGS_S) -o $@ $<
