project (Tutorial)

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
#set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

set(CMAKE_EXE_LINKER_FLAGS "-g")  ## Debug
set(MYLIBS "-g")
#set(MYLIBS "-lgstapp-1.0 -lgstbase-1.0 -lgstreamer-1.0")
set(MYLIBS "${MYLIBS} -lgstreamer-0.10 -lgstapp-0.10 -lgstbase-0.10")  
set(MYLIBS "${MYLIBS} -lgobject-2.0 -lgmodule-2.0 -pthread -lgthread-2.0 -pthread -lglib-2.0 -lxml2")
set(MYLIBS "${MYLIBS} -pthread -std=c++11 -fpermissive")

include_directories(/usr/include/gstreamer-0.10)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/i386-linux-gnu/glib-2.0/include/)
include_directories(/usr/lib/glib-2.0/include)
include_directories(/usr/include/libxml2)

set  (APP_NAME record)

#set (HEADERS  )

list(APPEND SRC  test_gst.cpp)
#list(APPEND SRC  )
#list(APPEND SRC  )
#list(APPEND SRC  )
#list(APPEND SRC  )
#list(APPEND SRC  )

add_executable(${APP_NAME}  ${SRC} ${HEADERS})
target_link_libraries(${APP_NAME} ${MYLIBS})
#target_link_libraries(${APP_NAME} ${OpenCV_LIBS} ${MYLIBS})