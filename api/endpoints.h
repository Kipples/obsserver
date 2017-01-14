#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include "helper.h"
#include "scenes.h"
#include "outputs.h"
#include "sources.h"
#include "users.h"
#include "rtmp.h"
#include "stream_keys.h"
#include "layout.h"

static struct _u_endpoint api_endpoints[] = {
  API_ENDPOINT(GET, "/scenes", scenes_GET),
  API_ENDPOINT(GET, "/scenes/:name", scenes_name_GET),
  API_ENDPOINT(POST, "/scenes/:name", scenes_name_POST),
  API_ENDPOINT(PUT, "/scenes/:name/active", scenes_name_active_PUT),
  API_ENDPOINT(GET, "/outputs", outputs_GET),
  API_ENDPOINT(GET, "/outputs/:name", outputs_name_GET),
  API_ENDPOINT(PUT, "/outputs/:name/active", outputs_name_active_PUT),
  API_ENDPOINT(DELETE, "/outputs/:name/active", outputs_name_active_DELETE),
  API_ENDPOINT(GET, "/sources", sources_GET),
  API_ENDPOINT(GET, "/sources/:name", sources_name_GET),
  API_ENDPOINT(POST, "/sources/:name", sources_name_POST),
  API_ENDPOINT_AUTH(GET, "/on_publish", on_publish_GET, rtmp_auth_function),
  API_ENDPOINT(GET, "/stream_keys", stream_keys_GET),
  API_ENDPOINT(GET, "/stream_keys/:name", stream_keys_GET),
  API_ENDPOINT(POST, "/stream_keys", stream_keys_POST),
  API_ENDPOINT(GET, "layout", layout_GET),
  API_ENDPOINT(POST, "layout", layout_POST),
  API_ENDPOINT(PUT, "layout/reflow", layout_reflow_PUT)
};


#endif
