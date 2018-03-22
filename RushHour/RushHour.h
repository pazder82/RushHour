#pragma once

#include "resource.h"

// Game constants
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define RUNINWINDOW   TRUE
#define ROTATESPEED   40.0
#define MOVE_SPEED    1.0
#define GLOWSPEED     3.0

/* Macros */
#define REMOVE_DEFAULT_CONSTRUCTOR(x) x() = delete;
#define REMOVE_COPY_CONSTRUCTOR(x) x(const x&) = delete;
#define REMOVE_MOVE_CONSTRUCTOR(x) x(x&&) = delete;
#define REMOVE_ASSIGNMENT_CONSTRUCTOR(x) x& operator=(const x&) = delete;
#define REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x) x& operator=(x &&) = delete;

// Remove default constructors...
#define RDC(x) REMOVE_DEFAULT_CONSTRUCTOR(x) REMOVE_COPY_CONSTRUCTOR(x) REMOVE_MOVE_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x)
#define RDC_D(x) REMOVE_COPY_CONSTRUCTOR(x) REMOVE_MOVE_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x)

struct Coords_t {
	int x;
	int z;
};
