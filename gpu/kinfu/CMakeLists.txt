set(SUBSYS_NAME gpu_kinfu)
set(SUBSYS_PATH gpu/kinfu)
set(SUBSYS_DESC "Kinect Fusion implementation")
set(SUBSYS_DEPS common gpu_containers gpu_utils gpu_features visualization io)

set(build FALSE)
PCL_SUBSYS_OPTION(build ${SUBSYS_NAME} ${SUBSYS_DESC} ON)
PCL_SUBSYS_DEPEND(build ${SUBSYS_NAME} DEPS ${SUBSYS_DEPS})
PCL_SET_SUBSYS_INCLUDE_DIR(${SUBSYS_NAME} ${SUBSYS_PATH})
mark_as_advanced("BUILD_${SUBSYS_NAME}")

if (build)
	#find_package(OpenCV QUIET)
	set(OpenCV_FOUND 0)

	if(OpenCV_FOUND)
		add_definitions(-DHAVE_OPENCV)	
		include_directories(${OpenCV_INCLUDE_DIRS})
	else()
		remove_definitions(-DHAVE_OPENCV)
	endif()

	FILE(GLOB incs include/pcl/gpu/kinfu/*.h*)
	FILE(GLOB srcs src/*.cpp src/*.h*)
	FILE(GLOB cuda src/cuda/*.cu src/cuda/*.h*)
	#FILE(GLOB tsdf src/cuda/tsdf_volume.cu)
	#FILE(GLOB est src/cuda/estimate_tranform.cu)
			
	source_group("Source Files\\cuda" FILES ${cuda} )    
	source_group("Source Files" FILES ${srcs} )		
	
	set(LIB_NAME pcl_${SUBSYS_NAME})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CUDA_INCLUDE_DIRS})			

	if (UNIX OR APPLE)
		set (CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}  "-Xcompiler;-fPIC;")			
	endif()
	
	if(NOT UNIX OR APPLE)
		add_definitions(-DPCLAPI_EXPORTS)
	endif()
	
	if(TRUE)
		#list(REMOVE_ITEM cuda ${est})
		#CUDA_COMPILE(est_objs ${est})		
				   
		set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} "--ftz=true;--prec-div=false;--prec-sqrt=false") 
		CUDA_COMPILE(cuda_objs ${cuda})		
		
		#LIST(APPEND cuda ${est})
		#LIST(APPEND cuda_objs ${est_objs})
		
	else()		
		list(REMOVE_ITEM cuda ${tsdf})
		CUDA_COMPILE(cuda_objs ${cuda})		
		set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}  "--ftz=true;--prec-div=false;--prec-sqrt=false") 
		CUDA_COMPILE(tsdf_obj ${tsdf})
	
		LIST(APPEND cuda ${tsdf})
		LIST(APPEND cuda_objs ${tsdf_obj})
	endif()
	 
	PCL_ADD_LIBRARY(${LIB_NAME} ${SUBSYS_NAME} ${srcs} ${incs} ${cuda} ${cuda_objs})
	target_link_libraries(${LIB_NAME} pcl_gpu_utils pcl_gpu_containers pcl_gpu_octree)
			
	if(OpenCV_FOUND)
		target_link_libraries(${LIB_NAME} ${OpenCV_LIBS})
	endif()
	 
	set(EXT_DEPS "")
	#set(EXT_DEPS CUDA)
	PCL_MAKE_PKGCONFIG(${LIB_NAME} ${SUBSYS_NAME} "${SUBSYS_DESC}" "${SUBSYS_DEPS}" "${EXT_DEPS}" "" "" "")

	# Install include files
	PCL_ADD_INCLUDES(${SUBSYS_NAME} ${SUBSYS_PATH} ${incs})
	
	add_subdirectory(test)
	add_subdirectory(tools)
endif()


