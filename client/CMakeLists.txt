cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#自动设置项目名称
project(client VERSION 0.1 LANGUAGES CXX)

#添加子模块
add_subdirectory(log)

set(MODULE_LIST)

#设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#打开相应的UIC MOC RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#引入QT库
set(CMAKE_PREFIX_PATH "D:/Packages/Qt/6.2.1/mingw81_64/lib/cmake")
include_directories(${log_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/utils)
include_directories(${PROJECT_SOURCE_DIR})

set(DIR_SRC_UI 
	./assets.qrc
	./mainwindow.ui
	./userwindow.ui
	./shopcarwindow.ui
	./adminwindow.ui
	)

set(DIR_SRC 
	${DIR_SRC_UI}
	./client.h
	./client.cpp
	./handler.h
	./handler.cpp
	./qtgui.h
	./qtgui.cpp
	./socket.h
	./socket.cpp
	./utils/dataType.h
	./utils/singleton.h
	./utils/utils.h
	./utils/utils.cpp
	)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)

#添加构建目标
#yurzi
add_library(yurzi STATIC ${DIR_SRC})
add_dependencies(yurzi log)
target_link_libraries(yurzi PUBLIC Qt6::Widgets)
target_link_libraries(yurzi PUBLIC Qt6::Network)
target_link_libraries(yurzi PUBLIC log)

#client
add_executable(${PROJECT_NAME} main.cpp ${DIR_SRC})
add_dependencies(${PROJECT_NAME} yurzi)

target_link_libraries(${PROJECT_NAME} PRIVATE yurzi)

#测试
add_executable(testGui ./tests/testGui.cpp)
target_link_libraries(testGui PRIVATE yurzi)

add_executable(testSocket ./tests/testSocketer.cpp)
target_link_libraries(testSocket PRIVATE yurzi)

add_executable(testLog ./tests/testlog.cpp)
target_link_libraries(testLog  PRIVATE yurzi)

add_executable(testController ./tests/testController.cpp ./assets.qrc)
target_link_libraries(testController  PRIVATE yurzi)

