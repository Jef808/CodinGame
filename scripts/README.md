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

Continuing with the above example, suppose the subdirectory's *CMakeLists.txt* file defines the main target as

``` cmake
add_executable(${CMAKE_PROJECT_NAME}_main main.cpp someHeader.cpp)
```

Then we define a custom target by adding

``` cmake
find_package(Python3 COMPONENTS Interpreter Development)

add_custom_target(${CMAKE_PROJECT_NAME}_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -d ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_PROJECT_NAME}_main
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -d ${CMAKE_CURRENT_BINARY_DIR}"
)
```

Now in the root's *CMakeLists.txt*, we simply add

``` cmake
project(subProject)
add_subdirectory(subProject)
```
    
(notice the usage of the CMAKE_PROJECT_NAME in the subproject's *CMakeLists.txt* file).
Now, running the usual
 
 ```bash
cmake -S . -B build
cmake --build build
 ```

a the root directory will create a *subProject_bundled.cpp* file in the *Root/build/subProject/* directory.

See the *example* directory for the above example.
