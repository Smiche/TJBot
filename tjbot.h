/*
 * TJBot
 * Copyright (c) 2016 Annihil
 * github.com/Annihil/TJBot
 */

#ifndef __TJBOT_H__
#define __TJBOT_H__

#ifdef __linux__
#define _GNU_SOURCE

#include <dlfcn.h>
#include <sys/mman.h>
#include <link.h>

#elif _WIN32
#include <windows.h>
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define BOT_ON_COMMAND "+jumpbot"
#define BOT_OFF_COMMAND "-jumpbot"
#define AUTO_ON_COMMAND "+autojump"
#define AUTO_OFF_COMMAND "-autojump"
#define ORIGIN_ON_COMMAND "+origin"
#define ORIGIN_OFF_COMMAND "-origin"
#define ORIGIN_SET_COMMAND "originset"
#define ORIGIN_GET_COMMAND "originget"
#define ANGLE_COMMAND "angle"
#define START_DUMP_COMMAND "startdump"
#define STOP_DUMP_COMMAND "stopdump"
#define VIEW_HACK_ON_COMMAND "+viewhack"
#define VIEW_HACK_OFF_COMMAND "-viewhack"
#define MS_PRINT_COMMAND "ms_print"
#define MS_RESET_COMMAND "ms_reset"
#define PS_PRINT_ON_COMMAND "+ps_print"
#define PS_PRINT_OFF_COMMAND "-ps_print"
#define SPRAY_COMMAND "spray"

#define ASM_JMP 0xe9
#define ASM_RETN 0xc3

// offsets
// client = 2.60b
#ifdef __linux__
#define DLOPEN 0x0804b830
#define DLSYM 0x0804adb0
#define MOUSE_X 0x0907d28c
#define KEY_FORWARD 0x08cc26e0
#define KEY_BACK 0x08cc26f8
#define KEY_MOVELEFT 0x08cc2740
#define KEY_MOVERIGHT 0x08cc2758
#define KEY_UP 0x08cc27a0
#elif _WIN32
#define CG_OFFSET 0x01DBEAB0
#define MOUSE_X 0x013EEA8C
#define KEY_FORWARD (0x00835A08 + 4 * 4)
#define KEY_BACK (0x00835A20 + 4 * 4)
#define KEY_MOVELEFT (0x00835A50 + 4 * 4)
#define KEY_MOVERIGHT (0x00835A68 + 4 * 4)
#define KEY_UP (0x00835AC8 + 4 * 4)
#endif

// cgame = etjump and other mods generaly use those values
#define PREDICTED_PS_OFFSET 0x48c20
#define ORIGIN_X 5 * 4
#define ORIGIN_Y 6 * 4
#define ORIGIN_Z 7 * 4
#define VELOCITY_X 8 * 4
#define VELOCITY_Y 9 * 4
#define VELOCITY_Z 10 * 4
#define VIEW_X 45 * 4
#define VIEW_Y 44 * 4
#define GROUND_ENTITY_NUM 20 * 4

struct tjbot_s {
    void *cgame_handle;
    unsigned int cgame_address;
    char cgame_filename[1024];
    void (*orig_cg_dllEntry)(int (*)(int, ...));
    int (*orig_cg_vmMain)(int, int, int, int, int, int, int, int, int, int, int, int, int);
    int (*orig_syscall)(int, ...);
    float *mouse_x;
    float *orig_x;
    float *orig_y;
    float *orig_z;
    float *vel_x;
    float *vel_y;
    float *vel_z;
    float *view_x;
    float *view_y;
    int *ground_ent_num;

    int jumpbot_enable;
    int autojump_enable;
    int origin_enable;
    int ps_print_enable;
    int orig_say;
    unsigned int disable_color_say;
    long origin_x;
    long origin_y;
    FILE *dump;
    int view_hack_enable;
    float max_speed;
    unsigned long frame_count;
};

static struct tjbot_s tjbot;

// syscalls
#define CG_PRINT 0
#define CG_ARGC 8
#define CG_ARGV 9
#define CG_SENDCONSOLECOMMAND 17
#define CG_ADDCOMMAND 18
#define CG_R_RENDERSCENE 69
#define CG_GETCURRENTCMDNUMBER 82
#define CG_GETUSERCMD 83

enum {
    CG_INIT,
    CG_SHUTDOWN,
    CG_CONSOLE_COMMAND,
    CG_DRAW_ACTIVE_FRAME
};

#define MAX_STRING_CHARS 1024

typedef enum {
    qfalse, qtrue
} qboolean;

typedef struct usercmd_s {
    int _pad[5];
    signed char forwardmove, rightmove;
    char __pad[3];
} usercmd_t;

#define ENTITYNUM_NONE 1023

typedef struct {
    char _pad[36];
    float viewaxis[3][3];
    char __pad[356];
} refdef_t;

#endif /* __TJBOT_H__ */
