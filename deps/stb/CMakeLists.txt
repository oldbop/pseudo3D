project(STB)

add_library(stb
  include/stb/stb_image.h
  src/stb_image.c
)

target_include_directories(stb PUBLIC include/)
