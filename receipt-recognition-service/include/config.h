#pragma once

#define STR(s) #s
#define XSTR(s) STR(s)

#define VERSION XSTR(1.0.0)
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#ifdef DEBUG
#define AWS_REGION "eu-central-1"
#endif

