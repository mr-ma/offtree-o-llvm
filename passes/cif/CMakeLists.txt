add_library(CIFPass MODULE
    CloneInlineFunctionPass.cpp
)

target_compile_features(CIFPass PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(CIFPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
