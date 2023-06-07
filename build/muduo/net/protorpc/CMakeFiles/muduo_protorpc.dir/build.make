# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zk/package/muduo-master

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zk/package/muduo-master/build

# Include any dependencies generated for this target.
include muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/compiler_depend.make

# Include the progress variables for this target.
include muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/progress.make

# Include the compile flags for this target's objects.
include muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/flags.make

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/flags.make
muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o: ../muduo/net/protorpc/RpcChannel.cc
muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zk/package/muduo-master/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o -MF CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o.d -o CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o -c /home/zk/package/muduo-master/muduo/net/protorpc/RpcChannel.cc

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.i"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zk/package/muduo-master/muduo/net/protorpc/RpcChannel.cc > CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.i

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.s"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zk/package/muduo-master/muduo/net/protorpc/RpcChannel.cc -o CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.s

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/flags.make
muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o: ../muduo/net/protorpc/RpcServer.cc
muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zk/package/muduo-master/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o -MF CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o.d -o CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o -c /home/zk/package/muduo-master/muduo/net/protorpc/RpcServer.cc

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/muduo_protorpc.dir/RpcServer.cc.i"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zk/package/muduo-master/muduo/net/protorpc/RpcServer.cc > CMakeFiles/muduo_protorpc.dir/RpcServer.cc.i

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/muduo_protorpc.dir/RpcServer.cc.s"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zk/package/muduo-master/muduo/net/protorpc/RpcServer.cc -o CMakeFiles/muduo_protorpc.dir/RpcServer.cc.s

# Object files for target muduo_protorpc
muduo_protorpc_OBJECTS = \
"CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o" \
"CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o"

# External object files for target muduo_protorpc
muduo_protorpc_EXTERNAL_OBJECTS =

lib/libmuduo_protorpc.a: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcChannel.cc.o
lib/libmuduo_protorpc.a: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/RpcServer.cc.o
lib/libmuduo_protorpc.a: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/build.make
lib/libmuduo_protorpc.a: muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zk/package/muduo-master/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library ../../../lib/libmuduo_protorpc.a"
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && $(CMAKE_COMMAND) -P CMakeFiles/muduo_protorpc.dir/cmake_clean_target.cmake
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/muduo_protorpc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/build: lib/libmuduo_protorpc.a
.PHONY : muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/build

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/clean:
	cd /home/zk/package/muduo-master/build/muduo/net/protorpc && $(CMAKE_COMMAND) -P CMakeFiles/muduo_protorpc.dir/cmake_clean.cmake
.PHONY : muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/clean

muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/depend:
	cd /home/zk/package/muduo-master/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zk/package/muduo-master /home/zk/package/muduo-master/muduo/net/protorpc /home/zk/package/muduo-master/build /home/zk/package/muduo-master/build/muduo/net/protorpc /home/zk/package/muduo-master/build/muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : muduo/net/protorpc/CMakeFiles/muduo_protorpc.dir/depend

