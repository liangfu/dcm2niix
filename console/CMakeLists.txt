project(dcm2nii)

set(PROGRAMS dcm2nii)

include_directories(${VTK_INCLUDE_DIRS})

add_executable(dcm2nii
  main_console.cpp
  nii_dicom.cpp
  nifti1_io_core.cpp
  nii_ortho.cpp
  nii_dicom_batch.cpp)

find_package(zlib)

# if(ZLIB_FOUND)
	target_link_libraries(dcm2nii z)
# else(ZLIB_FOUND)
#   ADD_DEFINITIONS(-DmyDisableZlib)
# endif(ZLIB_FOUND)

install(TARGETS ${PROGRAMS} DESTINATION bin)

