cmake_minimum_required (VERSION 3.3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(DIR_COMMON_OUTPUT 
	${CMAKE_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}
)

include(CompilerRuntime)
include(VcpkgIntegration)
project(xash-ms LANGUAGES CXX)
include(PlatformNaming)

list(APPEND FILE_SOURCES 
	"sources/application.cpp"
	"sources/main.cpp"
	"sources/timer.cpp"
	"sources/utils.cpp"
	"sources/socket.cpp"
	"sources/config_manager.cpp"
	"sources/config_data.cpp"
	"sources/server_list.cpp"
	"sources/server_entry.cpp"
	"sources/event_loop.cpp"
	"sources/libevent_wrappers.cpp"
	"sources/net_address.cpp"
	"sources/infostring_data.cpp"
	"sources/request_handler.cpp"
	"sources/binary_input_stream.cpp"
	"sources/binary_output_stream.cpp"
	"sources/admin_command_handler.cpp"
	"sources/packet_types/client_query_request.cpp"
	"sources/packet_types/client_query_response.cpp"
	"sources/packet_types/server_challenge_request.cpp"
	"sources/packet_types/server_challenge_response.cpp"
	"sources/packet_types/server_append_request.cpp"
	"sources/packet_types/admin_challenge_response.cpp"
	"sources/packet_types/admin_command_request.cpp"
)

add_executable(${PROJECT_NAME} ${FILE_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE 
	"sources"
	"sources/packet_types"
)

if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		_CRT_SECURE_NO_WARNINGS=1 # disable CRT warnings
	)

	# debug mode compiler flags
	target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/Od>) # disable optimizing at all
	# enable "Edit and Continue" MSVC feature
	target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>) 
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:
		/INCREMENTAL 
		/SAFESEH:NO
	>)

	# release mode compiler flags
	target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/GL>) # enable whole program optimization
	target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2>) # enable optimizing to maximize perfomance
else()
endif()

# enable static runtime linking
set_compiler_runtime(${PROJECT_NAME} STATIC)

# link dependency libraries
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(argparse CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)

find_package(Libevent CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libevent::core libevent::extra)

find_package(RapidJSON CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE rapidjson)

find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp::cryptopp)

if(BUILD_WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# get current git commit short hash & branch name
execute_process(COMMAND "git" "describe" "--always" "--dirty" "--abbrev=7"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE VCS_CURRENT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND "git" "rev-parse" "--abbrev-ref" "HEAD"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE VCS_CURRENT_BRANCH_NAME
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(VCS_CURRENT_COMMIT_HASH)
	message(STATUS "Commit hash: ${VCS_CURRENT_COMMIT_HASH}")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		BUILD_COMMIT_HASH="${VCS_CURRENT_COMMIT_HASH}"
	)
else()
	message(STATUS "Failed to get current Git commit hash")
endif()

if(VCS_CURRENT_BRANCH_NAME)
	message(STATUS "Branch name: ${VCS_CURRENT_BRANCH_NAME}")
	target_compile_definitions(${PROJECT_NAME} PRIVATE 
		BUILD_BRANCH_NAME="${VCS_CURRENT_BRANCH_NAME}"
	)
else()
	message(STATUS "Failed to get current Git branch name")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${DIR_COMMON_OUTPUT}
    LIBRARY_OUTPUT_DIRECTORY ${DIR_COMMON_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY ${DIR_COMMON_OUTPUT}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# copy compiled binaries to install directory
install(TARGETS ${PROJECT_NAME}
	DESTINATION "${CMAKE_INSTALL_PREFIX}"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	    GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE 
)
