macro(qtvsg_exe exe)
  if(${ARGC} EQUAL 1)
    set(src ${exe}.cpp)
  else()
    set(src ${ARGN})
  endif()

  add_executable(${exe} ${src})

  target_compile_definitions(${exe} PRIVATE vsgXchange_FOUND)

  target_include_directories(${exe} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

  target_link_libraries(${exe}
    vsg::vsg
    vsgImGui::vsgImGui
    vsgXchange::vsgXchange
    vsgQt
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    spnav
    fmt
    spdlog
  )
endmacro()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildsha1.h
         ${CMAKE_CURRENT_BINARY_DIR}/DummyNonExistingFile.h

  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/GetGitSha1.py ${CMAKE_CURRENT_BINARY_DIR}/buildsha1.h
)  

set_property(SOURCE buildsha1.cpp
  APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/buildsha1.h)

qtvsg_exe(qtvsgviewer
  qtvsgviewer.cpp
  mainwindow.cpp
  myapp.cpp
  widget3d.cpp
  buildsha1.cpp
)

install(TARGETS qtvsgviewer
  RUNTIME DESTINATION bin)
