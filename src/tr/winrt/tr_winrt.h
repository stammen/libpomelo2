/**
 * Copyright (c) 2014,2015 NetEase, Inc. and other Pomelo contributors
 * MIT Licensed.
 */

#ifndef TR_WINRT_H
#define TR_WINRT_H

#include <pomelo_trans.h>

#ifdef __cplusplus
extern "C" {
#endif
    pc_transport_plugin_t* pc_tr_winrt_trans_plugin();
#ifdef __cplusplus
}
#endif

#define TR_WINRT_RESP ("{\"msg\": \"dummy msg\"")



#endif
