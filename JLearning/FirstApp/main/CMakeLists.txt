# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.
# changing the structure of CMakeLists file 
idf_component_register(SRCS "tasks.c" "main.c" "ConfigStr.c"
                    INCLUDE_DIRS ".")
