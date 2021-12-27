cmake_minimum_required(VERSION 3.5)

#自动设置项目名称
project(log VERSION 0.1 LANGUAGES CXX)


#设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#打开相应的UIC MOC RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#引入QT库
set(CMAKE_PREFIX_PATH "D:/Packages/Qt/6.2.1/mingw81_64/lib/cmake")

#设置头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#设置源代码路径
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src DI_SRC)

#设置目标生成静态库
add_library(log STATIC ${DI_SRC})

#添加qt依赖
find_package(Qt6 COMPONENTS Core REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
