add_executable( sobel sobel.cpp )
target_link_libraries( sobel ${OPENCL_LIBRARIES} )

configure_file(sobel_image.cl ${CMAKE_CURRENT_BINARY_DIR}/sobel_image.cl COPYONLY)
