# macro to install a test executable (need add_test() to call it)
macro (merlin_test dir exe_name)
	set(source_files "")
	foreach(arg IN ITEMS ${ARGN})
		list(APPEND source_files ${dir}/${arg})
	endforeach()
	add_executable(${exe_name} ${source_files})

	# To keep the directory structure, we must set the output_name, add a rule to make the directory
	set_target_properties( ${exe_name} PROPERTIES OUTPUT_NAME ${dir}/${exe_name} )
	set_target_properties( ${exe_name} PROPERTIES COMPILE_FLAGS -g )
	add_custom_command(TARGET ${exe_name} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${dir})
	target_link_libraries(${exe_name} merlin)

	IF(COVERAGE)
		target_compile_options(${exe_name} PRIVATE -fprofile-arcs -ftest-coverage)
		SET_TARGET_PROPERTIES(${exe_name} PROPERTIES LINK_FLAGS "-fprofile-arcs ${CMAKE_EXE_LINKER_FLAGS}")
	ENDIF(COVERAGE)

endmacro (merlin_test)

# macro to install a test python script (need add_test() to call it)
macro (merlin_test_py dir script_name)
	add_custom_target(${script_name} ALL)
	add_custom_command(TARGET ${script_name}
	                   DEPENDS ${dir}/${script_name}
	                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${script_name} ${CMAKE_CURRENT_BINARY_DIR}/${dir}/${script_name})
endmacro (merlin_test_py)

# test with an extended timeout
macro (add_test_t name)
	add_test(${name} ${ARGN})
	set_tests_properties(${name} PROPERTIES TIMEOUT ${TEST_TIMEOUT})
endmacro (add_test_t)

add_custom_target(TestDataFiles ALL)
add_custom_command(TARGET TestDataFiles
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)
add_custom_command(TARGET TestDataFiles
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/outputs)

merlin_test_py(BasicTests have_python.py)
add_test_t(have_python BasicTests/have_python.py)

merlin_test(BasicTests bunch_test bunch_test.cpp)
add_test_t(bunch_test BasicTests/bunch_test)

merlin_test(BasicTests bunch_io_test bunch_io_test.cpp)
add_test_t(bunch_io_test BasicTests/bunch_io_test)

merlin_test(BasicTests landau_test landau_test.cpp)
merlin_test_py(BasicTests landau_test.py)
add_test_t(landau_test.py BasicTests/landau_test.py)

merlin_test(BasicTests aperture_test aperture_test.cpp)
add_test_t(aperture_test BasicTests/aperture_test)

merlin_test(BasicTests collimate_particle_process_test collimate_particle_process_test.cpp)
add_test_t(collimate_particle_process_test BasicTests/collimate_particle_process_test)

merlin_test(OpticsTests lhc_optics_test lhc_optics_test.cpp)
add_test_t(lhc_optics_test OpticsTests/lhc_optics_test)

merlin_test(OpticsTests lhc_fft_tune_test lhc_fft_tune_test.cpp)
add_test_t(lhc_fft_tune_test OpticsTests/lhc_fft_tune_test)

merlin_test(ScatteringTests cu50_test cu50_test.cpp)
merlin_test_py(ScatteringTests cu50_test.py)
add_test_t(cu50_test.py_1e7 ScatteringTests/cu50_test.py 0 10000000)
add_test_t(cu50_test.py_1e7_sixtrack ScatteringTests/cu50_test.py 0 10000000 sixtrack)
#add_test_t(cu50_test.py_1e8 ScatteringTests/cu50_test.py 0 100000000) # more thorough test
#add_test_t(cu50_test.py_1e8_sixtrack ScatteringTests/cu50_test.py 0 100000000 sixtrack) # more thorough test

merlin_test(ScatteringTests lhc_collimation_test lhc_collimation_test.cpp)
merlin_test_py(ScatteringTests lhc_collimation_test.py)
add_test_t(lhc_collimation_test.py_1e4 ScatteringTests/lhc_collimation_test.py 0 10000)
#add_test_t(lhc_collimation_test.py_1e5 ScatteringTests/lhc_collimation_test.py 0 100000)

merlin_test(ScatteringTests rand_test rand_test.cpp)


find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set( MEMORYCHECK_COMMAND_OPTIONS " --tool=memcheck --leak-check=yes --show-reachable=yes --trace-children=yes --suppressions=${CMAKE_CURRENT_BINARY_DIR}/data/python.supp")
INCLUDE(Dart)


