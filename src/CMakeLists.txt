add_executable(qtvsgviewer
  qtvsgviewer.cpp)

# Add qt6 modules to qtvsgviewer
target_link_libraries(qtvsgviewer PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  )
