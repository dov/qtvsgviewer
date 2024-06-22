macro(qtvsg_exe exe)
  if(${ARGC} EQUAL 1)
    set(src ${exe}.cpp)
  else()
    set(src ${ARGN})
  endif()

  add_executable(${exe} ${src})

  target_compile_definitions(${exe} PRIVATE vsgXchange_FOUND)

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
  )
endmacro()

qtvsg_exe(qtvsgviewer)

install(TARGETS qtvsgviewer
  RUNTIME DESTINATION bin)
