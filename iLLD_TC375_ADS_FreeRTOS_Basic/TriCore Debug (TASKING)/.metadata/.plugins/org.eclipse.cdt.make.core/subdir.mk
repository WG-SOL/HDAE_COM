################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.cpp" 

C_SRCS += \
"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c" 

COMPILED_SRCS += \
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src" 

CPP_DEPS += \
"./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d" 

C_DEPS += \
"./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d" 

OBJS += \
".metadata/.plugins/org.eclipse.cdt.make.core/specs.o" 


# Each subdirectory must supply rules for building sources it contributes
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src":"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc37x "-fC:/Users/USER/Documents/project2/iLLD_TC375_ADS_FreeRTOS_Basic/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc37x -Y0 -N0 -Z0 -o "$@" "$<"
".metadata/.plugins/org.eclipse.cdt.make.core/specs.o":".metadata/.plugins/org.eclipse.cdt.make.core/specs.src" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src":"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.cpp" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2004 -D__CPU__=tc37x "-fC:/Users/USER/Documents/project2/iLLD_TC375_ADS_FreeRTOS_Basic/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc37x -Y0 -N0 -Z0 -o "$@" "$<"

clean: clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core

clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core:
	-$(RM) ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.o ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.src

.PHONY: clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core

