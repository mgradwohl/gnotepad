add_executable(GnotePadSmoke smoke/smoke.cpp)

target_link_libraries(GnotePadSmoke PRIVATE Qt6::Core)

target_compile_features(GnotePadSmoke PRIVATE cxx_std_23)

add_test(NAME gnotepad_smoke COMMAND GnotePadSmoke)
