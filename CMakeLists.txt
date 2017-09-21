cmake_minimum_required(VERSION 2.8)

project(strange)

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})

option(DEMO "Set to ON if targeting DEMO Build" ${DEMO})

option(DATAS "Set to ON to copy Data in build folder (for direct play)" ${DATAS})

if(PANDORA)
	add_definitions(-DPANDORA)
	add_definitions(-DBIGGER)
endif()

if(DEMO)
	add_definitions(-DDEMO_VERSION)
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DLINUX=1)
endif()

set(GAME_SOURCE_FILES
src/cards.cpp
src/combat.cpp
src/combat_display.cpp
src/combat_init.cpp
src/combat_sim.cpp
src/combat_weapons.cpp
src/endgame.cpp
src/font.cpp
src/gfx.cpp
src/interface.cpp
src/is_fileio.cpp
src/main.cpp
src/modconfig.cpp
src/sdl_iface.cpp
src/sdl_main.cpp
src/sound.cpp
src/sprite_init.cpp
src/sprites.cpp
src/starmap.cpp
src/starmap_encounters.cpp
src/starmap_init.cpp
src/starmap_inventory.cpp
src/startgame.cpp
src/textstr.cpp
src/sdl_gfx.cpp
src/sdl_sound.cpp
src/cifm.cpp
)

find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)

set(THE_LIBS ${SDL_LIBRARY} ${SDL_MIXER_LIBRARY})

add_definitions("-Wno-write-strings")

add_executable(strange ${GAME_SOURCE_FILES})
include_directories(strange "src" ${SDL_INCLUDE_DIR} ${SDL_MIXER_INCLUDE_DIR})
target_link_libraries(strange ${THE_LIBS})

if(DATAS)
	if(DEMO)
	add_custom_command(TARGET strange POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/demo $<TARGET_FILE_DIR:strange>)
else()
	add_custom_command(TARGET strange POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/full $<TARGET_FILE_DIR:strange>)
	endif()
	if(PANDORA)
		add_custom_command(TARGET strange POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_file
			${CMAKE_SOURCE_DIR}/fnt7x8.pcx $<TARGET_FILE_DIR:strange>)
	endif()

endif()