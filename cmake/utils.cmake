cmake_minimum_required(VERSION 3.16)


if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()


function(message_color color_str text)

    if("${color_str}" STREQUAL "Red")
        message(STATUS "${Red}${text}${ColourReset}")

    elseif("${color_str}" STREQUAL "Green")
        message(STATUS "${Green}${text}${ColourReset}")

    elseif("${color_str}" STREQUAL "Yellow")
        message(STATUS "${Yellow}${text}${ColourReset}")

    elseif("${color_str}" STREQUAL "Cyan")
        message(STATUS "${Cyan}${text}${ColourReset}")

    elseif("${color_str}" STREQUAL "White")
        message(STATUS "${White}${text}${ColourReset}")
    endif()

endfunction()


