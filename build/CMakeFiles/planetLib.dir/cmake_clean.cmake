file(REMOVE_RECURSE
  "libplanetLib.pdb"
  "libplanetLib.a"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/planetLib.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
