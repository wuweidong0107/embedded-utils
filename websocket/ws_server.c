#include <stddef.h>
#include "ws_internal.h"
#include "ws_server.h"
#include "mongoose.h"

static const char *s_listen_on = NULL;
static const char *s_web_root = ".";
static int exiting = 0;

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_OPEN) {
        // c->is_hexdumping = 1;
    } else if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/websocket")) {
            // Upgrade to websocket. From now on, a connection is a full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
        } else if (mg_http_match_uri(hm, "/rest")) {
            // Serve REST response
            mg_http_reply(c, 200, "", "{\"result\": %d}\n", 123);
        } else {
            // Serve static files
            struct mg_http_serve_opts opts = {.root_dir = s_web_root};
            mg_http_serve_dir(c, ev_data, &opts);
        }
    } else if (ev == MG_EV_WS_MSG) {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
    }
    (void) fn_data;
}

static void task_ws_server(void *arg)
{
    struct mg_mgr mgr;  // Event manager
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_http_listen(&mgr, s_listen_on, fn, NULL);  // Create HTTP listener
    while (!exiting) {
        mg_mgr_poll(&mgr, 1000);             // Infinite event loop
    }
    mg_mgr_free(&mgr);
}

int ws_server_init(threadpool thpool, const char *url)
{
    int ret = 0;

    if (thpool == NULL || url == NULL)
        return -1;
    s_listen_on = url;

    if ((ret = ws_aw5808_init()))
        return ret;

    return thpool_add_work(thpool, task_ws_server, NULL);
}

void ws_server_exit(void)
{
    ws_aw5808_exit();
    exiting = 1;
}