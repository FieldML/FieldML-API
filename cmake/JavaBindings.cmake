FIND_PACKAGE(SWIG REQUIRED)
  FIND_PACKAGE(JNI)
  FIND_PACKAGE(Java)
  INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(io/src)
  INCLUDE_DIRECTORIES(core/src)
  INCLUDE(${SWIG_USE_FILE})

  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

  SET(CMAKE_SWIG_FLAGS "-Iio/src;-Icore/src;-package;fieldml.jni")
  SET(CMAKE_SWIG_OUTDIR "fieldml/jni")

  SET_SOURCE_FILES_PROPERTIES(jni/fieldml_api.i PROPERTIES CPLUSPLUS ON)
  SET_SOURCE_FILES_PROPERTIES(jni/fieldml_io.i PROPERTIES CPLUSPLUS ON)

  # If you change these version, also change the .i files.
  SWIG_ADD_MODULE(fieldml_jni_0.5 java jni/fieldml_api.i)
  SWIG_ADD_MODULE(fieldml_io_0.5 java jni/fieldml_io.i)
  SWIG_LINK_LIBRARIES(fieldml_jni_0.5 fieldml_api fieldml_io_api)
  SWIG_LINK_LIBRARIES(fieldml_io_0.5 fieldml_api fieldml_io_api)

  # Also make the jar file...
  SET(javafiles "fieldml/jni/FieldmlApiConstants.java;fieldml/jni/FieldmlApi.java;fieldml/jni/FieldmlApiJNI.java;fieldml/jni/FieldmlDataDescriptionType.java;fieldml/jni/FieldmlDataResourceType.java;fieldml/jni/FieldmlDataSourceType.java;fieldml/jni/FieldmlEnsembleMembersType.java;fieldml/jni/FieldmlHandleType.java;fieldml/jni/FieldmlIo.java;fieldml/jni/FieldmlIoJNI.java;fieldml/jni/SWIGTYPE_p_FmlBoolean.java;fieldml/jni/SWIGTYPE_p_FmlObjectHandle.java;fieldml/jni/SWIGTYPE_p_FmlSessionHandle.java;fieldml/jni/SWIGTYPE_p_int32_t.java")
  SET(classfiles "fieldml/jni/FieldmlApi.class;fieldml/jni/FieldmlApiConstants.class;fieldml/jni/FieldmlApiJNI.class;fieldml/jni/FieldmlDataDescriptionType.class;fieldml/jni/FieldmlDataDescriptionType\\$$SwigNext.class;fieldml/jni/FieldmlDataResourceType.class;fieldml/jni/FieldmlDataResourceType\\$$SwigNext.class;fieldml/jni/FieldmlDataSourceType.class;fieldml/jni/FieldmlDataSourceType\\$$SwigNext.class;fieldml/jni/FieldmlEnsembleMembersType.class;fieldml/jni/FieldmlEnsembleMembersType\\$$SwigNext.class;fieldml/jni/FieldmlHandleType.class;fieldml/jni/FieldmlHandleType\\$$SwigNext.class;fieldml/jni/FieldmlIoConstants.class;fieldml/jni/FieldmlIo.class;fieldml/jni/FieldmlIoConstants.class;fieldml/jni/FieldmlIoJNI.class;fieldml/jni/SWIGTYPE_p_FmlBoolean.class;fieldml/jni/SWIGTYPE_p_FmlObjectHandle.class;fieldml/jni/SWIGTYPE_p_FmlSessionHandle.class;fieldml/jni/SWIGTYPE_p_int32_t.class")

  ADD_CUSTOM_COMMAND(OUTPUT ${classfiles}
     COMMAND ${Java_JAVAC_EXECUTABLE} ARGS ${javafiles}
     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/jni/fieldml_apiJAVA_wrap.cxx)
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/jni/fieldml.jar
     COMMAND ${Java_JAR_EXECUTABLE} ARGS cf ${CMAKE_CURRENT_BINARY_DIR}/jni/fieldml.jar ${classfiles}
     DEPENDS ${classfiles})
  ADD_CUSTOM_TARGET(jnijar ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/jni/fieldml.jar)