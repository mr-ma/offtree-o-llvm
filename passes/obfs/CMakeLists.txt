add_library(ObfsPass MODULE
  BogusControlFlow.cpp
  CryptoUtils.cpp
  Flattening.cpp
  SplitBasicBlocks.cpp
  Substitution.cpp
  Utils.cpp
)

target_compile_features(ObfsPass PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(ObfsPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
