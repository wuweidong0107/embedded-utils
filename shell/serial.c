#include <stdlib.h>
#include <errno.h>
#include "log.h"
#include "shell.h"
#include "serial.h"
#include "device.h"

static int on_serial_receive(serial_t *serial, const uint8_t *buf, size_t len)
{
    int i;
    for (i=0; i<len; i++) {
        shell_printf("%i:%x\n",i, buf[i]);
    }
    return 0;
}

static struct serial_cbs serial_menu_cbs = {
    .on_receive = on_serial_receive,
};

int cmd_serial_list(int argc, char *argv[])
{
    int i;
    serial_t *aw;

    for (i=0; (aw=get_serial(i)) != NULL; i++)
        shell_printf("%d: %s\n", i, serial_id(aw));
    
    return 0;
}

int cmd_serial_write(int argc, char *argv[])
{
    int index;
    uint8_t data[128];
    int i,len;

    if (argc < 2)
        return -EINVAL;

    index = strtoul(argv[1], NULL, 10);
    serial_t *serial = get_serial(index);
    if (serial == NULL)
        return -EINVAL;

    for (i=2, len=0; i<argc && len<128; i++, len++) {
        data[len] = strtoul(argv[i], NULL, 16);
        shell_printf("%d:%x\n", len, data[len]);
    }
    len = len + 1;
    if (serial_write(serial, data, len) != len)
        log_info("%s", serial_errmsg(serial));
    
    return 0;
}

void serial_shell_init(void)
{
    int i;
    serial_t *serial;

    for (i=0; (serial=get_serial(i)) != NULL; i++) {
        serial_set_cbs(serial, &serial_menu_cbs);
    }
}