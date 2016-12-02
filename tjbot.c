/*
 * TJBot
 * Copyright (c) 2016 Annihil
 * github.com/Annihil/TJBot
 */

#include "tjbot.h"

// functions taken from Q3
static const char *CG_Argv(int arg) {
    static char buffer[MAX_STRING_CHARS];
    tjbot.orig_syscall(CG_ARGV, arg, buffer, sizeof(buffer));
    return buffer;
}

static float AngleNormalize360(float angle) {
    return (float) ((360.0 / 65536) * ((int) (angle * (65536 / 360.0)) & 65535));
}

static float AngleNormalize180(float angle) {
    angle = AngleNormalize360(angle);
    if (angle > 180.0)
        angle -= 360.0;
    return angle;
}

static float AngleDelta(float angle1, float angle2) {
    return AngleNormalize180(angle1 - angle2);
}

static inline float rad2deg(double a) {
    return (float) ((a * 180.0f) / M_PI);
}

static void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]) {
    out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
    out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
    out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
    out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
    out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
    out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
    out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
    out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
    out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

// hack functions
static void get_offsets() {
#ifdef __linux__
    void *cg_address = dlsym(tjbot.cgame_handle, "cg");
    if (!cg_address) {
        printf("\x1B[31mtjbot: cg symbol not found\x1B[0m\n");
        exit(EXIT_FAILURE);
    }
    printf("\x1B[33mtjbot: cg found @ 0x%x\x1B[0m\n", (unsigned int) cg_address - tjbot.cgame_address);
#elif _WIN32
    void* cg_address = (void*)((unsigned int)tjbot.cgame_handle + CG_OFFSET);
#endif
    unsigned int predictedPS_address = PREDICTED_PS_OFFSET + (unsigned int) cg_address;
    printf("\x1B[33mtjbot: predicted playerstate @ 0x%x\x1B[0m\n", predictedPS_address - tjbot.cgame_address);

    tjbot.orig_x = (float *) (predictedPS_address + ORIGIN_X);
    tjbot.orig_y = (float *) (predictedPS_address + ORIGIN_Y);
    tjbot.orig_z = (float *) (predictedPS_address + ORIGIN_Z);
    tjbot.vel_x = (float *) (predictedPS_address + VELOCITY_X);
    tjbot.vel_y = (float *) (predictedPS_address + VELOCITY_Y);
    tjbot.vel_z = (float *) (predictedPS_address + VELOCITY_Z);
    tjbot.view_x = (float *) (predictedPS_address + VIEW_X);
    tjbot.view_y = (float *) (predictedPS_address + VIEW_Y);
    tjbot.ground_ent_num = (int *) (predictedPS_address + GROUND_ENTITY_NUM);
    tjbot.mouse_x = (float *) MOUSE_X;
}

static void spray() {
    tjbot.disable_color_say = 43;
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^e___________ ____.^7__________        __\"\n");
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^e\\__    ___/|    |^7\\______   \\ _____/  |_\"\n");
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^e  |    |   |    | ^7|    |  _//  _ \\   __\\\"\n");
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^e  |    /\\__|    | ^7|    |   (  <_> )  |\"\n");
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^e  |____\\________| ^7|______  /\\____/|__|\"\n");
    tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say \"^7     git^e.^7io^e/^7v1tVH        \\/ by ^eA^7nnihil\"\n");
}

static void jump_bot() {
    int groundEntityNum = *tjbot.ground_ent_num;
    float accel_coef = groundEntityNum != ENTITYNUM_NONE ? 10.0f : 1.0f;

    usercmd_t cmd;
    int cmd_num = tjbot.orig_syscall(CG_GETCURRENTCMDNUMBER);
    tjbot.orig_syscall(CG_GETUSERCMD, cmd_num, &cmd);

    float vel_size = (float) sqrt(*tjbot.vel_x * *tjbot.vel_x + *tjbot.vel_y * *tjbot.vel_y);
#define MIN_SPEED 320
    if ((MIN_SPEED - MIN_SPEED / 125.0f * accel_coef) / vel_size * 1.1 > 1)
        return;
    float per_angle = AngleNormalize180(rad2deg(acos((MIN_SPEED - MIN_SPEED / 125.0f * accel_coef) / vel_size * 1.1)));
    float vel_angle = rad2deg(atan2(*tjbot.vel_y, *tjbot.vel_x));
    float accel_angle = rad2deg(atan2(-cmd.rightmove, cmd.forwardmove));

    /* magic '0.3' values are used to make sure that mouse is on the proper side
     * from the perfect angle */
#define MOUSE_SAFE_PAD 0.3
    if (cmd.forwardmove == 0) {
        if (cmd.rightmove > 0) // right halfbeat
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle - per_angle) -
                                      MOUSE_SAFE_PAD);
        else if (cmd.rightmove < 0) // left halfbeat
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle + per_angle) +
                                      MOUSE_SAFE_PAD);
    } else if (cmd.forwardmove > 0) {
        if (cmd.rightmove > 0) // right strafe
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle - per_angle) -
                                      MOUSE_SAFE_PAD);
        else if (cmd.rightmove < 0) // left strafe
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle + per_angle) +
                                      MOUSE_SAFE_PAD);
    } else if (cmd.forwardmove < 0) { // for viewhack
        if (cmd.rightmove > 0)
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle + per_angle) +
                                      MOUSE_SAFE_PAD);
        else if (cmd.rightmove < 0)
            *tjbot.mouse_x = (float) (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle - per_angle) -
                                      MOUSE_SAFE_PAD);
    }
}

// Automatically jumps if not in the air
static void autojump() {
    if (*tjbot.ground_ent_num != ENTITYNUM_NONE)
        *(int *) KEY_UP = qtrue;
    else
        *(int *) KEY_UP = qfalse;
}

// Help function
static void reset_keys() {
    *(int *) KEY_FORWARD = qfalse;
    *(int *) KEY_BACK = qfalse;
    *(int *) KEY_MOVELEFT = qfalse;
    *(int *) KEY_MOVERIGHT = qfalse;
}

// Tries to move to a given origin
static void origin_move() {
    *tjbot.mouse_x -= *tjbot.view_x;
    reset_keys();
    if (tjbot.origin_x - *tjbot.orig_x > 1)
        *(int *) KEY_FORWARD = qtrue;
    else if (tjbot.origin_x - *tjbot.orig_x < -1)
        *(int *) KEY_BACK = qtrue;
    if (tjbot.origin_y - *tjbot.orig_y > 1)
        *(int *) KEY_MOVELEFT = qtrue;
    else if (tjbot.origin_y - *tjbot.orig_y < -1)
        *(int *) KEY_MOVERIGHT = qtrue;
}

// Starts dumping (opens specified file for dumping)
static void start_dump() {
    if (tjbot.dump != NULL) {
        tjbot.orig_syscall(CG_PRINT, "Already dumping.\n");
        return;
    }
    if (tjbot.orig_syscall(CG_ARGC) != 2) {
        tjbot.orig_syscall(CG_PRINT, START_DUMP_COMMAND " <filename>\n");
        return;
    }
    if ((tjbot.dump = fopen(CG_Argv(1), "w")) == NULL) {
        tjbot.orig_syscall(CG_PRINT, "Error opening specified file.\n");
        return;
    }
}

char *va(char *format, ...) {
    static char buf[MAX_STRING_CHARS];
    va_list va;
    va_start(va, format);
    vsprintf(buf, format, va);
    va_end(va);
    return buf;
}

// replaced functions
int syscall(int cmd, ...) {
    int args[11];
    va_list va;
    va_start(va, cmd);
    for (int i = 0; i < sizeof(args) / sizeof(args[0]); i++)
        args[i] = va_arg(va, int);
    va_end(va);

    if (cmd == CG_R_RENDERSCENE && tjbot.view_hack_enable) {
        // 180 degs rotation matrix
        float mat[3][3] = {
                {-1.0f, 0.0f,  0.0f},
                {0.0f,  -1.0f, 0.0f},
                {0.0f,  0.0f,  1.0f}
        };

        refdef_t *rd = (refdef_t *) args[0];

        float res[3][3];
        MatrixMultiply(rd->viewaxis, mat, res);
        memcpy(rd->viewaxis, res, sizeof(rd->viewaxis));
        return tjbot.orig_syscall(cmd, rd);
    }
    return tjbot.orig_syscall(cmd, args[0], args[1], args[2], args[3], args[4],
                              args[5], args[6], args[7], args[8], args[9], args[10]);
}

void cg_dllEntry(int(*syscallptr)(int arg, ...)) {
    tjbot.orig_syscall = syscallptr;
    tjbot.orig_cg_dllEntry(syscall);
}

#ifdef __linux__

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    if (!strcmp(info->dlpi_name, tjbot.cgame_filename)) {
        tjbot.cgame_address = info->dlpi_addr;
        get_offsets();
    }
    return 0;
}

#endif

int cg_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8,
              int arg9, int arg10, int arg11) {
    switch (command) {
        case CG_INIT:
            tjbot.orig_syscall(CG_ADDCOMMAND, BOT_ON_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, BOT_OFF_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_ON_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_OFF_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_ON_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_OFF_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_SET_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_GET_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, ANGLE_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, START_DUMP_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, STOP_DUMP_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, VIEW_HACK_ON_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, VIEW_HACK_OFF_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, MS_PRINT_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, MS_RESET_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, PS_PRINT_ON_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, PS_PRINT_OFF_COMMAND);
            tjbot.orig_syscall(CG_ADDCOMMAND, SPRAY_COMMAND);

#ifdef __linux__
            tjbot.cgame_address = 0;
            dl_iterate_phdr(dl_iterate_callback, NULL);
#elif _WIN32
            get_offsets();
#endif
            break;
        case CG_SHUTDOWN:
            // Warning: file is closed for vid_restart too
            if (tjbot.dump) {
                fclose(tjbot.dump);
                tjbot.dump = NULL;
            }
        case CG_CONSOLE_COMMAND: {
            const char *cmd = CG_Argv(0);
            if (!strcmp(cmd, BOT_ON_COMMAND)) {
                tjbot.jumpbot_enable = 1;
                return qtrue;
            } else if (!strcmp(cmd, BOT_OFF_COMMAND)) {
                tjbot.jumpbot_enable = 0;
                return qtrue;
            } else if (!strcmp(cmd, AUTO_ON_COMMAND)) {
                tjbot.autojump_enable = 1;
                return qtrue;
            } else if (!strcmp(cmd, AUTO_OFF_COMMAND)) {
                *(int *) KEY_UP = qfalse;
                tjbot.autojump_enable = 0;
                return qtrue;
            } else if (!strcmp(cmd, "say") && !tjbot.disable_color_say) {
                if (tjbot.orig_say) {
                    tjbot.orig_say = 0;
                    break;
                }
                const char *msg = CG_Argv(1);
                char new_cmd[MAX_STRING_CHARS] = "say ";
                char *p = new_cmd + 4;

                int space = 1;
                // make sure we've got always enough space for "^0x \n\0"
                while (*msg != '\0' && p + 5 - new_cmd < sizeof(new_cmd)) {
                    if (*msg == ' ') {
                        space = 1;
                        *p++ = *msg++;
                        continue;
                    }
                    if (space == 1) {
                        *p++ = '^';
                        *p++ = 'e';
                        space = 2;
                    } else if (space == 2) {
                        *p++ = '^';
                        *p++ = '7';
                        space = 0;
                    }
                    *p++ = *msg++;
                }
                *p++ = '\n';
                *p = '\0';

                tjbot.orig_say = 1;
                tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, new_cmd);
                return qtrue;
            } else if (!strcmp(cmd, ORIGIN_ON_COMMAND)) {
                tjbot.origin_enable = 1;
                return qtrue;
            } else if (!strcmp(cmd, ORIGIN_OFF_COMMAND)) {
                reset_keys();
                tjbot.origin_enable = 0;
                return qtrue;
            } else if (!strcmp(cmd, ORIGIN_SET_COMMAND)) {
                tjbot.origin_x = strtol(CG_Argv(1), NULL, 10);
                tjbot.origin_y = strtol(CG_Argv(2), NULL, 10);
                return qtrue;
            } else if (!strcmp(cmd, ORIGIN_GET_COMMAND)) {
                tjbot.orig_syscall(CG_PRINT, va("origin: %f %f %f\n", *tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z));
                return qtrue;
            } else if (!strcmp(cmd, ANGLE_COMMAND)) {
                float a = strtof(CG_Argv(1), NULL);
                *tjbot.mouse_x += a - *tjbot.view_x;;
                return qtrue;
            } else if (!strcmp(cmd, START_DUMP_COMMAND)) {
                start_dump();
                return qtrue;
            } else if (!strcmp(cmd, STOP_DUMP_COMMAND)) {
                if (tjbot.dump) {
                    fclose(tjbot.dump);
                    tjbot.dump = NULL;
                }
                return qtrue;
            } else if (!strcmp(cmd, VIEW_HACK_ON_COMMAND)) {
                tjbot.view_hack_enable = 1;
                return qtrue;
            } else if (!strcmp(cmd, VIEW_HACK_OFF_COMMAND)) {
                tjbot.view_hack_enable = 0;
                return qtrue;
            } else if (!strcmp(cmd, MS_PRINT_COMMAND)) {
                tjbot.orig_syscall(CG_SENDCONSOLECOMMAND,
                                   va("orig_say ^7(^eTJ^7Bot) my max speed is ^e%.2f ^7UPS\n", tjbot.max_speed));
                return qtrue;
            } else if (!strcmp(cmd, MS_RESET_COMMAND)) {
                tjbot.max_speed = 0.0f;
                return qtrue;
            } else if (!strcmp(cmd, PS_PRINT_ON_COMMAND)) {
                tjbot.ps_print_enable = 1;
                return qtrue;
            } else if (!strcmp(cmd, PS_PRINT_OFF_COMMAND)) {
                tjbot.ps_print_enable = 0;
                return qtrue;
            } else if (!strcmp(cmd, SPRAY_COMMAND)) {
                spray();
                return qtrue;
            }
            break;
        }
        default:
            break;
    }

    int res = tjbot.orig_cg_vmMain(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

    switch (command) {
        case CG_DRAW_ACTIVE_FRAME: {
            // max speed
            float speed = (float) sqrt(*tjbot.vel_x * *tjbot.vel_x + *tjbot.vel_y * *tjbot.vel_y);
            if (speed > tjbot.max_speed)
                tjbot.max_speed = speed;
        }

            if (tjbot.jumpbot_enable)
                jump_bot();
            if (tjbot.autojump_enable)
                autojump();
            if (tjbot.origin_enable)
                origin_move();
            if (tjbot.dump)
                fprintf(tjbot.dump, "view: %f %f, origin: %f %f %f, velocity: %f %f %f\n", *tjbot.view_x, *tjbot.view_y,
                        *tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z, *tjbot.vel_x, *tjbot.vel_y, *tjbot.vel_z);
            if (tjbot.ps_print_enable)
                tjbot.orig_syscall(CG_PRINT, va("view: %f %f, origin : %f %f %f, velocity : %f %f %f\n", *tjbot.view_x,
                                                *tjbot.view_y,
                                                *tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z, *tjbot.vel_x, *tjbot.vel_y,
                                                *tjbot.vel_z));
            if (tjbot.frame_count >= 112500) { // 15 min @ 125fps
                spray();
                tjbot.frame_count = 0;
            }
            tjbot.frame_count++;
            if (tjbot.disable_color_say > 0)
                tjbot.disable_color_say--;
            break;
        default:
            break;
    }
    return res;
}

// hooked functions
#ifdef __linux__

void *tjbot_dlopen(const char *filename, int flag) {
    printf("\ntjbot_dlopen() %s loaded\n", filename);
    void *res = dlopen(filename, flag);
    if (filename && res && strstr(filename, "cgame.mp.i386.so")) {
        strcpy(tjbot.cgame_filename, filename);
        tjbot.cgame_handle = res;
    }
    return res;
}

void *tjbot_dlsym(void *handle, const char *symbol) {
    void *res = dlsym(handle, symbol);
    if (handle == tjbot.cgame_handle) {
        if (!strcmp(symbol, "dllEntry")) {
            tjbot.orig_cg_dllEntry = (void (*)(int(*)(int, ...))) res;
            return (void *) cg_dllEntry;
        } else if (!strcmp(symbol, "vmMain")) {
            tjbot.orig_cg_vmMain = (int (*)(int, int, int, int, int, int, int, int, int, int, int, int, int)) res;
            return (void *) cg_vmMain;
        }
    }
    return res;
}

void __attribute__((constructor)) tjbot_init() {
    mprotect((void *) (DLOPEN & 0xfffff000), 4096, PROT_READ | PROT_WRITE | PROT_EXEC); // dlopen hook
    *(unsigned char *) DLOPEN = ASM_JMP;
    *(unsigned long *) (DLOPEN + 1) = (unsigned long) tjbot_dlopen - DLOPEN - 5;

    mprotect((void *) (DLSYM & 0xfffff000), 4096, PROT_READ | PROT_WRITE | PROT_EXEC); // dlsym hook
    *(unsigned char *) DLSYM = ASM_JMP;
    *(unsigned long *) (DLSYM + 1) = (unsigned long) tjbot_dlsym - DLSYM - 5;
}

#elif _WIN32
#define SIZE 6
BYTE oldBytesLLA[SIZE] = { 0 };
BYTE JMPLLA[SIZE] = { 0 };
HMODULE WINAPI tjbot_LoadLibraryA(LPCTSTR lpFileName) {
    memcpy(LoadLibraryA, oldBytesLLA, SIZE);

    HINSTANCE hInst = LoadLibraryA(lpFileName);
    if (lpFileName && hInst && strstr(lpFileName, "cgame_mp_x86.dll")) {
        strcpy(tjbot.cgame_filename, lpFileName);
        tjbot.cgame_handle = hInst;
    }

    memcpy(LoadLibraryA, JMPLLA, SIZE);
    return hInst;
}

BYTE oldBytesGPA[SIZE] = { 0 };
BYTE JMPGPA[SIZE] = { 0 };
FARPROC WINAPI tjbot_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    memcpy(GetProcAddress, oldBytesGPA, SIZE);

    FARPROC fRet = GetProcAddress(hModule, lpProcName);
    if (hModule == tjbot.cgame_handle) {
        if (!strcmp(lpProcName, "dllEntry")) {
            tjbot.orig_cg_dllEntry = (void(*)(int(*)(int, ...))) fRet;
            fRet = (FARPROC)cg_dllEntry;
        }
        else if (!strcmp(lpProcName, "vmMain")) {
            tjbot.orig_cg_vmMain = (int(*)(int, int, int, int, int, int, int, int, int, int, int, int, int)) fRet;
            fRet = (FARPROC)cg_vmMain;
        }
    }

    memcpy(GetProcAddress, JMPGPA, SIZE);
    return fRet;
}

void hijack(LPVOID newFunction, LPVOID origFunction, BYTE oldBytes[SIZE], BYTE JMP[SIZE]) {
    BYTE tempJMP[SIZE] = { ASM_JMP, 0x90, 0x90, 0x90, 0x90, ASM_RETN };
    memcpy(JMP, tempJMP, SIZE);
    DWORD JMPSize = ((DWORD)newFunction - (DWORD)origFunction - 5);
    DWORD dwProtect;
    VirtualProtect((LPVOID)origFunction, SIZE, PAGE_EXECUTE_READWRITE, &dwProtect);
    memcpy(oldBytes, origFunction, SIZE);
    memcpy(&JMP[1], &JMPSize, 4);
    memcpy(origFunction, JMP, SIZE);
}

BOOL WINAPI DllMain(HMODULE hDll, DWORD dwReason, PVOID pvReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        hijack(tjbot_LoadLibraryA, LoadLibraryA, oldBytesLLA, JMPLLA);
        hijack(tjbot_GetProcAddress, GetProcAddress, oldBytesGPA, JMPGPA);
    }
    return 1;
}
#endif