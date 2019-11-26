add_library(DIFPass MODULE
    DeleteInlineFunctionPass.cpp
)

target_compile_features(DIFPass PRIVATE cxx_range_for cxx_auto_type)

set_target_properties(DIFPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
