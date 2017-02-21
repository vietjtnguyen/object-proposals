cmake_minimum_required( VERSION 3.1 )
project( object-proposals CXX )

macro( add_include_dep INCL )
  find_path( DEP_INCL_${INCL} ${INCL} )
  if( NOT DEP_INCL_${INCL} )
    message( SEND_ERROR "Unable to find include \"${INCL}\"" )
  else()
    message( STATUS "Found include \"${INCL}\" at \"${DEP_INCL_${INCL}}\"" )
  endif()
  list( APPEND DEP_INCLS ${DEP_INCL_${INCL}} )
endmacro()

macro( add_library_dep LIB )
  find_library( DEP_LIB_${LIB} ${LIB} )
  if( NOT DEP_LIB_${LIB} )
    message( SEND_ERROR "Unable to find library \"${LIB}\"" )
  else()
    message( STATUS "Found library \"${LIB}\" at \"${DEP_LIB_${LIB}}\"" )
  endif()
  list( APPEND DEP_LIBS ${DEP_LIB_${LIB}} )
endmacro()

add_include_dep( "argagg/argagg.hpp" )

add_include_dep( "dlib/image_transforms.h" )
add_library_dep( dlib )

add_include_dep( "opencv2/opencv.hpp" )
add_library_dep( opencv_core )
add_library_dep( opencv_highgui )
add_library_dep( opencv_imgcodecs )
add_library_dep( opencv_imgproc )
add_library_dep( opencv_videoio )
add_library_dep( opencv_video )

add_library_dep( jpeg )
add_library_dep( png )

add_include_dep( "time.h" )

add_executable(
  find-object-proposals
  "${CMAKE_CURRENT_SOURCE_DIR}/bin/find-object-proposals.cpp"
)
set_target_properties(
  find-object-proposals
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS
      "-g;-Wall;-Werror"
    INCLUDE_DIRECTORIES "${DEP_INCLS}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LINK_LIBRARIES "${DEP_LIBS}"
    OUTPUT_NAME find-object-proposals
)
