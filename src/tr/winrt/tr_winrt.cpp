/**
 * Copyright (c) 2014,2015 NetEase, Inc. and other Pomelo contributors
 * MIT Licensed.
 */

#include <stdlib.h>
#include <assert.h>

#include <pc_lib.h>
#include <string>
#include <memory>
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>

#include "tr_winrt.h"
#include "pr_msg.h"


#include <cpprest/ws_client.h>
using namespace web;
using namespace web::websockets::client;
using namespace Concurrency::streams;

class WinRTWebSocket {
public:
    WinRTWebSocket()
    : m_ws()
    {};

    websocket_callback_client m_ws;
    pc_JSON* route_to_code;
    pc_JSON* code_to_route;
    pc_JSON* dict_ver;
    pc_JSON* proto_ver;
    pc_JSON* client_protos;
    pc_JSON* server_protos;
};


typedef struct winrt_transport_s {

    pc_transport_t base;
    pc_client_t* client;

    uv_buf_t(*pr_msg_encoder)(const pc_JSON *, const pc_JSON *, const pc_msg_t* msg);
    pc_msg_t(*pr_msg_decoder)(const pc_JSON *, const pc_JSON *, const uv_buf_t* buf);

    WinRTWebSocket* websocket;
} winrt_transport_t;

#ifdef __cplusplus
extern "C" {
#endif

static int winrt_init(pc_transport_t* trans, pc_client_t* client)
{
    winrt_transport_t* d_tr = (winrt_transport_t*) trans;
    assert(d_tr);

    d_tr->client = client;

    return PC_RC_OK;
}

static int winrt_connect(pc_transport_t* trans, const char* host, int port, const char* handshake_opt)
{
    winrt_transport_t* tr = (winrt_transport_t* )trans;
    assert(tr);


#if 0
    std::string url("ws://");
    url += host;
    url += ":";
    url += std::to_string(port);
#else
    std::string url("ws://echo.websocket.org");
#endif
    std::wstring w_url(url.begin(), url.end());


    tr->websocket->m_ws.connect(w_url.c_str()).then([tr]()
    { /* We've finished connecting. */ 
    
        pc_trans_fire_event(tr->client, PC_EV_CONNECTED, NULL, NULL);
    });

    // set receive handler
    tr->websocket->m_ws.set_message_handler([](websocket_incoming_message msg)
    {
        msg.extract_string().then([](std::string body) {
            std::string foo = body;
        });
    });

    return PC_RC_OK;
}

static int winrt_send(pc_transport_t* trans, const char* route, unsigned int seq_num, const char* msg, unsigned int req_id, int timeout)
{
    winrt_transport_t* tr = (winrt_transport_t*)trans;

    assert(trans && route && msg && req_id != PC_INVALID_REQ_ID);

    pc_msg_t m;
    m.id = req_id;
    m.msg = msg;
    m.route = route;

    uv_buf_t buf = pr_default_msg_encoder(tr->websocket->code_to_route, tr->websocket->client_protos, &m);
    if (buf.len == (unsigned int)-1) {
        pc_lib_log(PC_LOG_ERROR, "tr_uv_tcp_send - encode msg failed, msg: %s, route: %s", msg, route);
        return PC_RC_ERROR;
    }

    uv_buf_t pkg_buf = pc_pkg_encode(PC_PKG_DATA, buf.base, buf.len);
    pc_lib_free(buf.base);


    if (pkg_buf.len == (unsigned int)-1) {
        pc_lib_log(PC_LOG_ERROR, "tr_uv_tcp_send - encode package failed");
        return PC_RC_ERROR;
    }

    winrt_transport_t* d_tr = (winrt_transport_t* )trans;
    assert(d_tr);

    concurrency::streams::producer_consumer_buffer<uint8_t> pcbuf;

    auto send_task = pcbuf.putn_nocopy((uint8_t*)&pkg_buf.base[0], pkg_buf.len).then([&](size_t length) {
        websocket_outgoing_message wom;
        wom.set_binary_message(pcbuf.create_istream(), length);
        return d_tr->websocket->m_ws.send(wom);
    }).then([](pplx::task<void> t)
    {
        try
        {
            t.get();
        }
        catch (const websocket_exception& ex)
        {
            std::cout << ex.what();
        }
    });
    send_task.wait();


    return PC_RC_OK;
}

static int winrt_disconnect(pc_transport_t* trans)
{
    winrt_transport_t* d_tr = (winrt_transport_t* )trans;
    assert(d_tr);

    d_tr->websocket->m_ws.close().then([d_tr]()
    { /* We've finished disconnecting. */
        pc_trans_fire_event(d_tr->client, PC_EV_DISCONNECT, NULL, NULL);
    });

    return PC_RC_OK;
}

static int winrt_cleanup(pc_transport_t* trans)
{
    return PC_RC_OK;
}

static void* winrt_internal_data(pc_transport_t* trans)
{
    return NULL;
}

static pc_transport_plugin_t* winrt_plugin(pc_transport_t* trans)
{
    return pc_tr_winrt_trans_plugin();
}

static int winrt_conn_quality(pc_transport_t* trans)
{
    return 0;
}

static pc_transport_t* winrt_trans_create(pc_transport_plugin_t* plugin)
{
    pc_transport_t* trans = (pc_transport_t* )pc_lib_malloc(sizeof(winrt_transport_t));

    trans->init = winrt_init;
    trans->connect = winrt_connect;
    trans->send = winrt_send;
    trans->disconnect = winrt_disconnect;
    trans->cleanup = winrt_cleanup;
    trans->internal_data = winrt_internal_data;
    trans->plugin = winrt_plugin;
    trans->quality = winrt_conn_quality;

    winrt_transport_t* d_tr = (winrt_transport_t*)trans;

    d_tr->websocket = new WinRTWebSocket();

    return trans;
}

#if 0
static void tcp__send_handshake(pc_transport_plugin_t* t)
{
    uv_buf_t buf;
    pc_JSON* sys;
    pc_JSON* body;

    char* data;
    int i;

    body = pc_JSON_CreateObject();
    sys = pc_JSON_CreateObject();

//    assert(tt->state == TR_UV_TCP_HANDSHAKEING);

    WinRTWebSocket* tt = t->
    assert((tt->proto_ver && tt->client_protos && tt->server_protos)
        || (!tt->proto_ver && !tt->client_protos && !tt->server_protos));

    assert((tt->dict_ver && tt->route_to_code && tt->code_to_route)
        || (!tt->dict_ver && !tt->route_to_code && !tt->code_to_route));

    if (tt->proto_ver) {
        pc_JSON_AddItemReferenceToObject(sys, "protoVersion", tt->proto_ver);
    }

    if (tt->dict_ver) {
        pc_JSON_AddItemReferenceToObject(sys, "dictVersion", tt->dict_ver);
    }

    pc_JSON_AddItemToObject(sys, "type", pc_JSON_CreateString(pc_lib_platform_type));
    pc_JSON_AddItemToObject(sys, "version", pc_JSON_CreateString(pc_lib_version_str()));

    pc_JSON_AddItemToObject(body, "sys", sys);

    if (tt->handshake_opts) {
        pc_JSON_AddItemReferenceToObject(body, "user", tt->handshake_opts);
    }

    data = pc_JSON_PrintUnformatted(body);
    buf = pc_pkg_encode(PC_PKG_HANDSHAKE, data, strlen(data));

    pc_lib_free(data);
    pc_JSON_Delete(body);

    wi = NULL;
    pc_mutex_lock(&tt->wq_mutex);
    for (i = 0; i < TR_UV_PRE_ALLOC_WI_SLOT_COUNT; ++i) {
        if (PC_PRE_ALLOC_IS_IDLE(tt->pre_wis[i].type)) {
            wi = &tt->pre_wis[i];
            PC_PRE_ALLOC_SET_BUSY(wi->type);
            break;
        }
    }

    if (!wi) {
        wi = (tr_uv_wi_t*)pc_lib_malloc(sizeof(tr_uv_wi_t));
        memset(wi, 0, sizeof(tr_uv_wi_t));
        wi->type = PC_DYN_ALLOC;
    }

    QUEUE_INIT(&wi->queue);
    TR_UV_WI_SET_INTERNAL(wi->type);

    wi->buf = buf;
    wi->seq_num = -1; /* internal data */
    wi->req_id = -1; /* internal data */
    wi->timeout = PC_WITHOUT_TIMEOUT; /* internal timeout */
    wi->ts = time(NULL); /* TODO: time() */

                         /*
                         * insert to head, because handshake req should be sent
                         * before any application data.
                         */
    QUEUE_INSERT_HEAD(&tt->write_wait_queue, &wi->queue);
    pc_mutex_unlock(&tt->wq_mutex);

    uv_async_send(&tt->write_async);
}
#endif

static void winrt_trans_release(pc_transport_plugin_t* plugin, pc_transport_t* trans)
{
    pc_lib_free(trans);
}

static pc_transport_plugin_t instance =
{
    winrt_trans_create,
    winrt_trans_release,
    NULL, /* on_register */
    NULL, /* on_deregister */
    PC_TR_NAME_WINRT_TCP
};

pc_transport_plugin_t* pc_tr_winrt_trans_plugin()
{
    return &instance;
}

#ifdef __cplusplus
}
#endif

