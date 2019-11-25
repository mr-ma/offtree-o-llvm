add_library(PrepPass MODULE
    PreparationPass.cpp
)

target_compile_features(PrepPass PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(PrepPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
