# target_compile_definitions(libcaos PUBLIC CAOS_CHECK_COVERAGE)

# target_compile_options(libcaos PRIVATE -fprofile-instr-generate -fcoverage-mapping)
# target_link_options(libcaos PRIVATE -fprofile-instr-generate -fcoverage-mapping)

#     add_compile_options(-fprofile-arcs -ftest-coverage)
#     add_link_options(--coverage)

# find_program(GCOVR_EXECUTABLE gcovr)
# find_program(LCOV_EXECUTABLE lcov)
# find_program(GENHTML_EXECUTABLE genhtml)

# if(GCOVR_EXECUTABLE)
#     add_custom_target(coverage
#         COMMAND ${GCOVR_EXECUTABLE}
#             --gcov-executable "llvm-cov-18 gcov"
#             --root ${CMAKE_SOURCE_DIR}
#             --exclude ".*/tests/.*"
#             --exclude ".*/_deps/.*"
#             --html --html-details -o ${CMAKE_BINARY_DIR}/coverage.html
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         DEPENDS ${PROJECT_NAME}_tests
#         COMMENT "Generating coverage report with gcovr..."
#     )
# elseif(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
#     add_custom_target(coverage
#         COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
#         COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
#         COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         DEPENDS ${PROJECT_NAME}_tests
#         COMMENT "Generating coverage report with lcov..."
#     )
# else()
#     message(WARNING "Code coverage tools not found. Install gcovr with: pip install gcovr")
# endif()
