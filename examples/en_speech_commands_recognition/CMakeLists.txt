cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS
    ../../components
    main/components       # ← buraya main/ altındaki components dizinini ekledik
    )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(speech_commands_recognition)
