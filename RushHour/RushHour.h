#pragma once

#include "resource.h"

// Game constants
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define RUNINWINDOW   TRUE

#define ROTATESPEED   40.0
#define MOVE_SPEED    1.0
#define GLOWSPEED     3.0

#define INFOPOSITIONX 2.0f
#define INFOPOSITIONY 5.0f

#define CAMINITPOSITION XMVectorSet(0.0f, 4.0f, -10.0f, 1.0f)
#define LIGHTPOSITION XMVectorSet(10.0f, 8.0f, 6.0f, 1.0f);
#define CARLIGHTINTENSITY 0.8f
#define AMBIENTCOLORINTENSITY 0.45f

/* Macros */
#define REMOVE_DEFAULT_CONSTRUCTOR(x) x() = delete;
#define REMOVE_COPY_CONSTRUCTOR(x) x(const x&) = delete;
#define REMOVE_MOVE_CONSTRUCTOR(x) x(x&&) = delete;
#define REMOVE_ASSIGNMENT_CONSTRUCTOR(x) x& operator=(const x&) = delete;
#define REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x) x& operator=(x &&) = delete;

// Remove default constructors...
#define RDC(x) REMOVE_DEFAULT_CONSTRUCTOR(x) REMOVE_COPY_CONSTRUCTOR(x) REMOVE_MOVE_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x)
#define RDC_D(x) REMOVE_COPY_CONSTRUCTOR(x) REMOVE_MOVE_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_CONSTRUCTOR(x) REMOVE_ASSIGNMENT_MOVE_CONSTRUCTOR(x)

struct Coords2i_t {
	int x;
	int z;
};

struct Coords3f_t {
	float x;
	float y;
	float z;
};
