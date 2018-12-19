#pragma once

// Check icons utf-8 code: https://fontawesome.com/cheatsheet
#include "IconsFontAwesome5.h"
#include "IconsFontAwesome5Brands.h"

#ifndef VKTUTO_OPENGL_MAJOR_VERSION
#define VKTUTO_OPENGL_MAJOR_VERSION 3
#endif // !VKTUTO_OPENGL_MAJOR_VERSION

#ifndef VKTUTO_OPENGL_MINOR_VERSION
#define VKTUTO_OPENGL_MINOR_VERSION 0
#endif // !VKTUTO_OPENGL_MINOR_VERSION

#ifndef VKTUTO_CLEAR_COLOR
#define VKTUTO_CLEAR_COLOR 0.45f, 0.55f, 0.60f, 1.00f
#endif // !VKTUTO_CLEAR_COLOR

#ifndef VKTUTO_WINDOW_WIDTH
#define VKTUTO_WINDOW_WIDTH 1280
#endif // !VKTUTO_WINDOW_WIDTH

#ifndef VKTUTO_WINDOW_HEIGHT
#define VKTUTO_WINDOW_HEIGHT 720
#endif // !VKTUTO_WINDOW_HEIGHT

#ifndef VKTUTO_WINDOW_TITLE
#define VKTUTO_WINDOW_TITLE "TEST"
#endif // !VKTUTO_WINDOW_TITLE


#ifndef VKTUTO_FONT_COMMON_DIRECTORY
#define VKTUTO_FONT_COMMON_DIRECTORY "../../misc/fonts/Noto_Sans_KR/"
#endif

#ifndef VKTUTO_FONT_NORMAL_PATH
#define VKTUTO_FONT_NORMAL_PATH VKTUTO_FONT_COMMON_DIRECTORY \
                                "NotoSansKR-Regular.otf"
#endif

#ifndef VKTUTO_FONT_BOLD_PATH
#define VKTUTO_FONT_BOLD_PATH VKTUTO_FONT_COMMON_DIRECTORY \
                              "NotoSansKR-Bold.otf"
#endif

#ifndef VKTUTO_FONT_THIN_PATH
#define VKTUTO_FONT_THIN_PATH VKTUTO_FONT_COMMON_DIRECTORY \
                              "NotoSansKR-Thin.otf"
#endif

#ifndef VKTUTO_FONT_ICON_COMMON_DIRECTORY
#define VKTUTO_FONT_ICON_COMMON_DIRECTORY "../../misc/fonts/FontAwesome_5.6.1/"
#endif

#ifndef VKTUTO_FONT_ICON_SOLID_PATH
#define VKTUTO_FONT_ICON_SOLID_PATH VKTUTO_FONT_ICON_COMMON_DIRECTORY \
                                    FONT_ICON_FILE_NAME_FAS
#endif

#ifndef VKTUTO_FONT_ICON_REGULAR_PATH
#define VKTUTO_FONT_ICON_REGULAR_PATH VKTUTO_FONT_ICON_COMMON_DIRECTORY \
                                      FONT_ICON_FILE_NAME_FAR
#endif

#ifndef VKTUTO_FONT_ICON_BRAND_PATH
#define VKTUTO_FONT_ICON_BRAND_PATH VKTUTO_FONT_ICON_COMMON_DIRECTORY \
                                    FONT_ICON_FILE_NAME_FAB
#endif