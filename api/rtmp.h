#ifndef RTMP_H
#define RTMP_H

#include "../obs_server.h"

int rtmp_auth_function(const struct _u_request *request,
		       struct _u_response *response,
		       void *user_data) {
  return U_OK;
}

int on_publish_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;

  const char *stream_name = u_map_get(request->map_url, "name");

  int rc = 401;

  const char *key;
  json_t *value;

  bool valid_stream = false;

  json_object_foreach(data->stream_keys, key, value) {
    if(strcmp(json_string_value(value), stream_name) == 0) {
      valid_stream = true; break;
    }
  }

  const char *timer_stream_key = json_string_value(json_object_get(data->global, "timer_stream_key"));
  if(strcmp(timer_stream_key, stream_name) == 0) valid_stream = true;

  const char *stats_stream_key = json_string_value(json_object_get(data->global, "stats_stream_key"));
  if(strcmp(stats_stream_key, stream_name) == 0) valid_stream = true;

  if(valid_stream) {
    rc = 200;

    obs_source_t *source = obs_get_source_by_name(stream_name);
    obs_data_t *d = obs_source_get_settings(source);
    obs_data_set_bool(d, "restart_on_activate", true);
    obs_source_update(source, d); // hack to get media_source to reload
    obs_data_set_bool(d, "restart_on_activate", false);
    obs_source_update(source, d); // hack to get media_source to reload
  }

  ulfius_set_empty_response(response, rc);

  return U_OK;
}



#endif /* RTMP_H */
