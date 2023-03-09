# Bundler.py

## Specifying sources

Include a *sources.txt* file alongside each subdirectory to specify which files will be included in the bundle.
For example, if the root directory has the form

``` 
Root
├── subproject
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
add_custom_target(main_bundled
  ALL
  COMMAND ${scripts_DIR}/bundler.py ${CMAKE_CURRENT_SOURCE_DIR}/sources.txt
  DEPENDS main
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/main_bundled.cpp"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)
```
    
This ensures that a *main_bundled.cpp* file automatically gets compiled from the sources declared in *sources.txt* every time the subproject is built (e.g. via *cmake --build <build-directory>*).

