# C-Bindings extra target
find_package(PythonInterp QUIET)
set(HAVE_Fortran_BINDINGS FALSE)
if (PYTHONINTERP_FOUND)
    set(HAVE_Fortran_BINDINGS TRUE)
    enable_language(Fortran)
    set(FIELDML_FORTRAN ${CMAKE_CURRENT_BINARY_DIR}/fieldml_fortran.f90)
    
    # Generate the file
    add_custom_command(OUTPUT ${FIELDML_FORTRAN}
        COMMAND ${PYTHON_EXECUTABLE} InterfaceGen.py ${FIELDML_FORTRAN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Fortran)
        
    # Compile module and lib
    SET(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fortran_modules)
    add_library(fieldml-fortran ${FIELDML_FORTRAN})
    
    # install stuff
    install(TARGETS fieldml-fortran
        EXPORT fieldml-api-config
        DESTINATION lib)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fortran_modules/
        DESTINATION include)
else()
    message(WARNING "No Python interpreter found. Unable to generate Fortran bindings for FieldML.")
endif()