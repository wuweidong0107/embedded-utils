#ifndef __AW5808_H__
#define __AW5808_H__

#include <stdint.h>
#include <ev.h>
#include "hidraw.h"
#include "serial.h"
#include "list.h"

enum aw5808_error_code {
    AW5808_ERROR_ARG            = -1, /* Invalid arguments */
    AW5808_ERROR_OPEN           = -2, /* Opening aw5808 device */
    AW5808_ERROR_QUERY          = -3, /* Querying aw5808 device attributes */
    AW5808_ERROR_CONFIGURE      = -4, /* Configuring aw5808 device attributes */
    AW5808_ERROR_IO_SERIAL      = -5, /* Reading/writing aw5808 device via serial */
    AW5808_ERROR_IO_HID         = -6, /* Reading/writing aw5808 device via hidraw */
    AW5808_ERROR_CLOSE          = -7, /* Closing aw5808 device */
};

typedef enum aw5808_mode {
    AW5808_MODE_I2S = 0,
    AW5808_MODE_USB = 1,
    AW5808_MODE_UNKNOWN = 2,
} aw5808_mode_t;

typedef enum aw5808_i2s_mode {
    AW5808_MODE_I2S_MASTER = 0,
    AW5808_MODE_I2S_SLAVE = 1,
    AW5808_MODE_I2S_UNKNOWN = 2,
} aw5808_i2s_mode_t;

typedef enum aw5808_connect_mode {
    AW5808_MODE_CONN_MULTI = 0,
    AW5808_MODE_CONN_SINGLE = 1,
    AW5808_MODE_CONN_UNKNOWN = 2,
} aw5808_connect_mode_t;

typedef struct aw5808_handle aw5808_t;

typedef struct aw5808_options {
    char serial[96];             /* optional */
    char usb[96];                 /* optional */
    aw5808_mode_t mode;             /* i2s/usb */
    struct ev_loop *loop;
} aw5808_options_t;

struct aw5808_client_ops {
    void (*on_get_config)(aw5808_t *aw, uint16_t firmware_ver, uint8_t mcu_ver, aw5808_mode_t mode, uint8_t rf_channel, uint8_t rf_power);
    void (*on_get_rfstatus)(aw5808_t *aw, const uint8_t is_connected, uint8_t pair_status);
    void (*on_notify_rfstatus)(aw5808_t *aw, uint8_t is_connected, uint8_t pair_status);
    void (*on_pair)(aw5808_t *aw);
    void (*on_set_mode)(aw5808_t *aw, aw5808_mode_t mode);
    void (*on_set_i2s_mode)(aw5808_t *aw, aw5808_i2s_mode_t mode);
    void (*on_set_connect_mode)(aw5808_t *aw, aw5808_connect_mode_t mode);
    void (*on_set_rfchannel)(aw5808_t *aw, uint8_t channel);
    void (*on_set_rfpower)(aw5808_t *aw, uint8_t power);
};

struct aw5808_client {
    char name[64];
    struct aw5808_client_ops *ops;
    struct list_head list;
};

aw5808_t *aw5808_new();
void aw5808_free(aw5808_t *aw);
int aw5808_open(aw5808_t *aw, aw5808_options_t *opt);
void aw5808_close(aw5808_t *aw);
int aw5808_get_config(aw5808_t *aw);
int aw5808_get_rfstatus(aw5808_t *aw);
int aw5808_reply_rfstatus_notify(aw5808_t *aw);
int aw5808_pair(aw5808_t *aw);
int aw5808_set_mode(aw5808_t *aw, aw5808_mode_t mode);
int aw5808_set_mode_sync(aw5808_t *aw, aw5808_mode_t mode, int timeout_us);
int aw5808_set_i2s_mode(aw5808_t *aw, aw5808_i2s_mode_t mode);
int aw5808_set_connect_mode(aw5808_t *aw, aw5808_connect_mode_t mode);
int aw5808_set_rfchannel(aw5808_t *aw, uint8_t channel);
int aw5808_set_rfpower(aw5808_t *aw, uint8_t power);
int aw5808_read_fw(aw5808_t *aw, uint8_t *buf, size_t len);
int aw5808_add_client(aw5808_t *aw, struct aw5808_client *client);
void aw5808_remove_client(aw5808_t *aw, struct aw5808_client *client);
int aw5808_mode(aw5808_t *aw);
const char *aw5808_id(aw5808_t *aw);
const char *aw5808_tostring(aw5808_t *aw);
/* Error Handling */
int aw5808_errno(aw5808_t *aw);
const char *aw5808_errmsg(aw5808_t *aw);

#endif
