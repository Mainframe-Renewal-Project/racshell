
cmake_minimum_required(VERSION 3.24)

project(racshell C CXX)

set(CXX_STANDARD 17)

add_subdirectory(sear)

add_library(
    racshell

    racshell/include/formatters/text_formatter.cpp
    racshell/include/formatters/json_formatter.cpp
)

target_include_directories(
    racshell PUBLIC
    racshell
    racshell/include
    externals/argparse/
    externals/nlohmann/
)

# Link racshell with the sear library target defined in sear/CMakeLists.txt
target_link_libraries(racshell PUBLIC sear)

# User commands
add_executable(adduser "racshell/src/commands/user/adduser.cpp")
target_link_libraries(adduser racshell -static)

add_executable(altuser "racshell/src/commands/user/altuser.cpp")
target_link_libraries(altuser racshell -static)

add_executable(deleteuser "racshell/src/commands/user/deleteuser.cpp")
target_link_libraries(deleteuser racshell -static)

add_executable(listuser "racshell/src/commands/user/listuser.cpp")
target_link_libraries(listuser racshell -static)

add_executable(searchusers "racshell/src/commands/user/searchusers.cpp")
target_link_libraries(searchusers racshell -static)

add_executable(compareusers "racshell/src/commands/user/compareusers.cpp")
target_link_libraries(compareusers racshell -static)

# Group commands
add_executable(addgroup "racshell/src/commands/group/addgroup.cpp")
target_link_libraries(addgroup racshell -static)

add_executable(altgroup "racshell/src/commands/group/altgroup.cpp")
target_link_libraries(altgroup racshell -static)

add_executable(deletegroup "racshell/src/commands/group/deletegroup.cpp")
target_link_libraries(deletegroup racshell -static)

add_executable(listgroup "racshell/src/commands/group/listgroup.cpp")
target_link_libraries(listgroup racshell -static)

add_executable(searchgroups "racshell/src/commands/group/searchgroups.cpp")
target_link_libraries(searchgroups racshell -static)

add_executable(comparegroups "racshell/src/commands/group/comparegroups.cpp")
target_link_libraries(comparegroups racshell -static)

add_executable(connect "racshell/src/commands/group/connect.cpp")
target_link_libraries(connect racshell -static)

add_executable(remove "racshell/src/commands/group/remove.cpp")
target_link_libraries(remove racshell -static)

# Dataset commands
add_executable(adddataset "racshell/src/commands/dataset/adddataset.cpp")
target_link_libraries(adddataset racshell -static)

add_executable(altdataset "racshell/src/commands/dataset/altdataset.cpp")
target_link_libraries(altdataset racshell -static)

add_executable(deletedataset "racshell/src/commands/dataset/deletedataset.cpp")
target_link_libraries(deletedataset racshell -static)

add_executable(listdataset "racshell/src/commands/dataset/listdataset.cpp")
target_link_libraries(listdataset racshell -static)

add_executable(searchdatasets "racshell/src/commands/dataset/searchdatasets.cpp")
target_link_libraries(searchdatasets racshell -static)

add_executable(permitdataset "racshell/src/commands/dataset/permitdataset.cpp")
target_link_libraries(permitdataset racshell -static)

add_executable(deletepermitdataset "racshell/src/commands/dataset/deletepermitdataset.cpp")
target_link_libraries(deletepermitdataset racshell -static)

# Resource commands
add_executable(addresource "racshell/src/commands/resource/addresource.cpp")
target_link_libraries(addresource racshell -static)

add_executable(altresource "racshell/src/commands/resource/altresource.cpp")
target_link_libraries(altresource racshell -static)

add_executable(deleteresource "racshell/src/commands/resource/deleteresource.cpp")
target_link_libraries(deleteresource racshell -static)

add_executable(listresource "racshell/src/commands/resource/listresource.cpp")
target_link_libraries(listresource racshell -static)

add_executable(searchresource "racshell/src/commands/resource/searchresource.cpp")
target_link_libraries(searchresource racshell -static)

add_executable(permitresource "racshell/src/commands/resource/permitresource.cpp")
target_link_libraries(permitresource racshell -static)

add_executable(deletepermitresource "racshell/src/commands/resource/deletepermitresource.cpp")
target_link_libraries(deletepermitresource racshell -static)

add_custom_target(
    racshell_commands
    DEPENDS
    listuser
    searchusers
    compareusers
    listgroup
    searchgroups
    comparegroups
    adduser
    addgroup
    deleteuser
    deletegroup
    altuser
    altgroup
    connect
    remove
    adddataset
    deletedataset
    listdataset
    searchdatasets
    listresource
    addresource
    altresource
    deleteresource
    searchresource
    permitdataset
    permitresource
    deletepermitdataset
    deletepermitresource
)