cmake_minimum_required(VERSION 3.21)
project(MusicPlayer VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Multimedia Network Sql Concurrent
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

# ── kissfft (vendored single-header) ──────────────────────────────
set(KISSFFT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/kissfft)
add_library(kissfft STATIC ${KISSFFT_DIR}/kiss_fft.c)
target_include_directories(kissfft PUBLIC ${KISSFFT_DIR})
set_source_files_properties(${KISSFFT_DIR}/kiss_fft.c PROPERTIES LANGUAGE C)

# ── Source files ──────────────────────────────────────────────────
add_executable(music
    src/main.cpp
    src/app/Application.cpp
    src/model/Track.h
    src/model/Playlist.h
    src/db/Database.cpp
    src/core/AudioEngine.cpp
    src/core/TagManager.cpp
    src/core/LibraryScanner.cpp
    src/core/LyricsManager.cpp
    src/network/JamendoService.cpp
    src/network/CoverArtService.cpp
    src/network/LyricsService.cpp
    src/ui/MainWindow.cpp
    src/ui/LibraryPanel.cpp
    src/ui/PlayerControls.cpp
    src/ui/VisualizationWidget.cpp
    src/ui/EqualizerWidget.cpp
    src/ui/LyricsWidget.cpp
    src/ui/TagEditorDialog.cpp
    src/ui/PlaylistPanel.cpp
    src/ui/OnlineSearchDialog.cpp
    resources/resources.qrc
)

target_include_directories(music PRIVATE src)
target_link_libraries(music PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Multimedia Qt6::Network Qt6::Sql Qt6::Concurrent
    kissfft
    ${TAGLIB_LIBRARIES}
)
target_include_directories(music PRIVATE ${TAGLIB_INCLUDE_DIRS})
