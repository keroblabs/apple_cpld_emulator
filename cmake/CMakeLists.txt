cmake_minimum_required(VERSION 3.16)

set(MODULE_NAME         apple_cpld)
set(RTL_MODEL_MMU       VMMU)
set(RTL_MODEL_IOU       VIOU)
set(RTL_MODEL           Vsystem)
set(RTL_MODEL_TB        VIOU_MMU_tb)

set(RTL_MMU_OBJ_DIR     ../rtl_MMU_v2/obj_dir)
set(RTL_IOU_OBJ_DIR     ../rtl_IOU_v2/obj_dir)
set(RTL_TB_OBJ_DIR      ../rtl_IOU_MMU_TB/obj_dir)
set(VERILATOR_INC       /home/marcio/Tools/verilator/include)

#find_package(SDL2 REQUIRED)

add_definitions(-D_USE_SCREEN_WIDTH=1280)
add_definitions(-D_USE_SCREEN_HEIGHT=800)
add_definitions(-DTIMA_USE_CONFIG)
add_definitions(-DMAC_OS_EMULATOR)
add_definitions(-D_USE_LINUX)
#add_definitions(-DSPLIT_CPLD)

add_compile_options(-Wno-sign-compare)


add_definitions("-Wall")

set(SOURCE_FILES        ../cpld/rtl_model.cpp
                        ../cpld/rtl_interface.cpp
                        #../self_test/rtl_self_test.cpp
                        #../self_test/rtl_self_test_MMU.cpp
                        #../self_test/rtl_self_test_CPLD.cpp

                        ../emulator/onboard/board.c
                        ../emulator/onboard/keyboard_controller.c
                        ../emulator/onboard/spi_controller.c
                        ../emulator/onboard/memory_controller.c
                        ../emulator/onboard/iomem_controller.c

                        ../emulator/rtl_cpu_65816/rtl_cpu_65816.c

                        ../emulator/rtl_video_ctrl/rtl_video.c
                        ../emulator/rtl_video_ctrl/ram_bus_hw.c
                        ../emulator/rtl_video_ctrl/sw_switch.c

                        ../emulator/slot_peripherals/hard_drive.c
                        ../emulator/slot_peripherals/disc_drive.c

                        ../emulator/disasm_65816/disasm_816.c

                        ../source/app_emulator.c
                        ../source/input_drv.c
                        ../source/filesys_drv.c

                        ../graphics/draw.c
                        ../graphics/graphics.c
                        ../graphics/font_4x6.c
                        ../graphics/font_8x8.c
                        ../graphics/font_12x16.c

                        ../sdl_library/sdl_input.c
                        ../sdl_library/sdl_main.c
                        ../sdl_library/sdl_mouse.c
                        ../sdl_library/sdl_window.c

                        ../drivers/lcd_driver.c
                        ../drivers/mouse_driver.c
                        ../drivers/keyraw_driver.c

                        ../system/system_timer.c
                        ../system/t_threads.c
                        ../system/heap_mngr.c

                        ../framework/buffer.c
                        ../framework/timer.c
                        )

include_directories(    ${VERILATOR_INC}
                        #${RTL_OBJ_DIR}
                        ${RTL_TB_OBJ_DIR}
                        #${RTL_MMU_OBJ_DIR}
                        #${RTL_IOU_OBJ_DIR}
                        ${SDL2_INCLUDE_DIR}
                        #/home/marcio/Projects/GIT_Dev/SDL/build/include
                        #/home/marcio/Projects/GIT_Dev/SDL/build/include/SDL2

                        ../
                        ../graphics
                        ../system
                        ../framework
                        ../drivers
                        ../sdl_library

                        ../emulator/rtl_video_ctrl
                        ../emulator/rtl_cpu_65816
                        ../emulator/onboard
                        ../emulator/slot_peripherals

                        ../emulator/rtl_cpu_65816
                        ../emulator/rtl_video_ctrl

                        ../emulator/disasm_65816
                        ../emulator

                        ../source
                        ../self_test
                        ../cpld

                        )

link_directories(       /home/marcio/Projects/GIT_Dev/SDL/build
                        #${RTL_MMU_OBJ_DIR}
                        #${RTL_IOU_OBJ_DIR}
                        #${RTL_OBJ_DIR}
                        ${RTL_TB_OBJ_DIR}
                                            )


project(${MODULE_NAME})
add_executable(${MODULE_NAME} ${SOURCE_FILES})
#target_link_libraries(${MODULE_NAME} ${RTL_MODEL_IOU} ${RTL_MODEL_MMU} verilated SDL2 m dl)
#target_link_libraries(${MODULE_NAME} ${RTL_MODEL} verilated SDL2 m dl)
target_link_libraries(${MODULE_NAME} ${RTL_MODEL_TB} verilated SDL2 m dl)

#add_custom_target   (   build_rtl_TB ALL
#                        COMMAND ./build.sh IOU_MMU_tb.v
#                        WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../rtl_IOU_MMU_TB
#                        COMMENT "Build RTL"
#                    )

#add_custom_target   (   build_rtl_MMU ALL
#                        COMMAND ./build.sh MMU.v
#                        WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../rtl_MMU
#                        COMMENT "Build RTL MMU"
#                    )

#add_custom_target   (   build_rtl_IOU ALL
#                        COMMAND ./build.sh IOU.v
#                        WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../rtl_IOU
#                        COMMENT "Build RTL IOU"
#                    )

#add_dependencies    (   ${MODULE_NAME} build_rtl_TB build_rtl_MMU build_rtl_IOU )

