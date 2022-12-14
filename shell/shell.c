#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <ev.h>
#include <readline/readline.h>
#include <unistd.h>
#include "log.h"
#include "shell.h"
#include "shell_internal.h"
#include "stdstring.h"
#include "device.h"

typedef int (*cmd_fn_t)(int argc, char *argv[]);
typedef struct {
    const char *name;
    cmd_fn_t func;
    const char *doc;
} command_t;

static command_t cmd_list[];
static struct {
    int argc;
    char **argv;
    bool mode;
} data;

static int cmd_help(int argc, char *argv[])
{
    int i=0;
    shell_printf("Avaliable command:\n");
    for (; cmd_list[i].name; i++) {
        char *tokens[1];
        string_split(cmd_list[i].name, "_", tokens, 1);
        if ((!strncmp("aw5808", tokens[0], strlen("aw5808")) && get_aw5808(0) == NULL) 
            || (!strncmp("serial", tokens[0],strlen("serial")) && get_serial(0) == NULL)) {
            free(tokens[0]);
        } else {
            free(tokens[0]);
            shell_printf("\t%-40s %s\n", cmd_list[i].name, cmd_list[i].doc);
        }
    }
    shell_printf("Exit by Ctrl+D.\n");
    return 0;
}

static command_t cmd_list[] = {
    { "aw5808_list", cmd_aw5808_list, "List available aw5808 device" },
    { "aw5808_getconfig [index]", cmd_aw5808_get_config, "Get aw5808 config" },
    { "aw5808_getrfstatus [index]", cmd_aw5808_get_rfstatus, "Get aw5808 RF status" },
    { "aw5808_pair [index]", cmd_aw5808_pair, "Pair aw5808 with headphone" },
    { "aw5808_setmode [index] <i2s|usb>", cmd_aw5808_set_mode, "Set aw5808 mode" },
    { "aw5808_seti2smode [index] <master|slave>", cmd_aw5808_set_i2s_mode, "Set aw5808 i2s mode" },
    { "aw5808_setconnmode [index] <multi|single>", cmd_aw5808_set_connect_mode, "Set aw5808 connect mode" },
    { "aw5808_setrfchannel [index] <1-8>", cmd_aw5808_set_rfchannel, "Set aw5808 RF channel" },
    { "aw5808_setrfpower [index] <1-16>", cmd_aw5808_set_rfpower, "Set aw5808 RF power" },
    { "serial_list", cmd_serial_list, "List available serial device" },
    { "serial_write <index> <data1 data2 ...>", cmd_serial_write, "Send hex data by serial" },
    { "usb_hid_enumerate", cmd_usb_hid_enumerate, "List all usb hid device" },
    { "usb_hid_list", cmd_usb_hid_list, "List available usb hid device" },
    { "usb_hid_write <index> <data1 data2 ...>", cmd_usb_hid_write, "Send hex data by usbhid" },
    { "help", cmd_help, "Disply help info" },
    { NULL, NULL, NULL},
};

static command_t *shell_find_command(const char *name)
{
    int i;

    for (i=0; cmd_list[i].name; i++) {
        char *tokens[2];
        size_t count;
        count = string_split(cmd_list[i].name, " ", tokens, 2);
        if(strcmp(name, tokens[0]) == 0)
            return (&cmd_list[i]);
        
        for(int j=0; j<count; j++)
            free(tokens[j]);
    }

    return NULL;
}

void shell_printf(const char *fmt, ...)
{
    va_list args;
    bool save_input;
    char *saved_line;
    int saved_point;

    save_input = !RL_ISSTATE(RL_STATE_DONE);
    if (save_input) {
        saved_point = rl_point;
        saved_line = rl_copy_text(0, rl_end);
        rl_save_prompt();
        rl_replace_line("", 0);
        rl_redisplay();
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (save_input) {
        rl_restore_prompt();
        rl_replace_line(saved_line, 0);
        rl_point = saved_point;
        rl_forced_update_display();
        free(saved_line);
    }
}

void shell_exec(int argc, char *argv[])
{
    int ret;
    command_t *cmd = shell_find_command(argv[0]);
    if (cmd) {
        ret = cmd->func(argc, argv);
        switch(ret) {
            case 0:
                break;
            case -ENOENT: 
                shell_printf("Unkown command!\n");
                cmd_help(0, NULL);
                break;
            case -EINVAL:
                shell_printf("Invalid command param\n");
                cmd_help(0, NULL);
                break;
            default:
                shell_printf("Command exec fail, ret=%d\n", ret);
        }
        return;
    }
    cmd_help(0, NULL);
    return;
}

int shell_init(void)
{
    int ret = 0;
    if ((ret = aw5808_shell_init()))
        return ret;
    if ((ret = serial_shell_init()))
        return ret;
    if ((ret = usb_shell_init()))
        return ret;
    return ret;
}

void shell_exit(void)
{
    aw5808_shell_exit();
    serial_shell_exit();
    usb_shell_exit();
}