
if(NOT "E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitinfo.txt" IS_NEWER_THAN "E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: 'E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "E:/Codes/C++/VulkanEngine/vendor/stb"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: 'E:/Codes/C++/VulkanEngine/vendor/stb'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "E:/Apps/Git/cmd/git.exe"  clone --no-checkout --depth 1 --no-single-branch --config "advice.detachedHead=false" "https://github.com/nothings/stb" "stb"
    WORKING_DIRECTORY "E:/Codes/C++/VulkanEngine/vendor"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/nothings/stb'")
endif()

execute_process(
  COMMAND "E:/Apps/Git/cmd/git.exe"  checkout origin/master --
  WORKING_DIRECTORY "E:/Codes/C++/VulkanEngine/vendor/stb"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'origin/master'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "E:/Apps/Git/cmd/git.exe"  submodule update --recursive --init 
    WORKING_DIRECTORY "E:/Codes/C++/VulkanEngine/vendor/stb"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: 'E:/Codes/C++/VulkanEngine/vendor/stb'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitinfo.txt"
    "E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: 'E:/Codes/C++/VulkanEngine/build/STB-prefix/src/STB-stamp/STB-gitclone-lastrun.txt'")
endif()

