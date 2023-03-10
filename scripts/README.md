# Bundler.py

## Specifying sources

Include a *sources.txt* file alongside each subdirectory to specify which files will be included in the bundle.
For example, if the root directory has the form

```
Root
├── CMakeLists.txt
├── ...
├── subProject
|   ├── CMakeLists.txt
|   ├── sources.txt
│   ├── helpers.h
│   ├── helpers.cpp
│   └── main.cpp
| ...
```

Then the subproject's *sources.txt* file should simply contain
``` text
helpers.h
helpers.cpp
main.cpp
```

## Cmake integration

Continuing with the above examle, suppose the subdirectory's *CMakeLists.txt* file defines the main target as

``` cmake
add_executable(main main.cpp someHeader.cpp)
```

Then we define a custom target by adding

``` cmake
find_package(Python3 COMPONENTS Interpreter Development)

add_custom_target(escape_the_cat_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o escapethecat_bundled -d ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS escape_the_cat
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/escape_the_cat_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o escapethecat_bundled -d ${CMAKE_CURRENT_BINARY_DIR}"
)
```

Now in the root's *CMakeLists.txt*, we simply add a line

``` cmake
add_subdirectory(subProject)
```
    
 and running the usual
 
 ```bash
cmake -S . -B build
cmake --build build
 ```

will create a *main_bundled.cpp* file in *Root/build/subProject*
