# Require libfuzzer specific flags
set(CMAKE_CXX_FLAGS "-O1 -gline-tables-only -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link -stdlib=libstdc++")
