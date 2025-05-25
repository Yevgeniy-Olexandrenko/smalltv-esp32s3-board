#pragma once

#include "board.h"
#include "tftespi.h"
#include <stdint.h>

#define PROJECT_NAME            "SmallTV"
#define PROJECT_VERSION         "1.0.0"
#define PROJECT_ICON            "📺"

#define WEBAPP_TITLE            (PROJECT_ICON " " PROJECT_NAME)
#define WEBAPP_PROJECT_HOME     "https://github.com/Yevgeniy-Olexandrenko/smalltv-esp32s3-board"

#define NETWORK_ACCESS_POINT    PROJECT_NAME
#define NETWORK_HOST_NAME       PROJECT_NAME

#define STORAGE_MSC_VENDORID    "ESP32"
#define STORAGE_MSC_PRODUCTID   PROJECT_NAME
#define STORAGE_MSC_PRODUCTREV  PROJECT_VERSION
