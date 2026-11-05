cmake_minimum_required(VERSION 3.22)
project(R2CharacterLcdTests VERSION 1.0 LANGUAGES C CXX)
if(NOT JUCE_SOURCE_DIR)
    message(FATAL_ERROR "Set JUCE_SOURCE_DIR to a JUCE checkout")
endif()
add_subdirectory("${JUCE_SOURCE_DIR}" juce)
juce_add_console_app(r2_character_lcd_tests PRODUCT_NAME "R2 LCD Tests")
target_sources(r2_character_lcd_tests PRIVATE CharacterLcdTests.cpp ../modules/R2JUCE/src/R2CharacterLcd.cpp)
target_compile_features(r2_character_lcd_tests PRIVATE cxx_std_17)
target_compile_definitions(r2_character_lcd_tests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(r2_character_lcd_tests PRIVATE juce::juce_gui_basics)
enable_testing()
add_test(NAME character_lcd COMMAND r2_character_lcd_tests)

add_executable(r2_sequence_port_tests SequencePortTests.cpp
    ../modules/R2JUCE/src/R2SequenceFile.cpp
    ../modules/R2JUCE/src/R2RcpFile.cpp
    ../modules/R2JUCE/src/R2WrdFile.cpp)
target_compile_features(r2_sequence_port_tests PRIVATE cxx_std_20)
add_test(NAME sequence_ports COMMAND r2_sequence_port_tests "${CMAKE_CURRENT_BINARY_DIR}")
