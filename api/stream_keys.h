#ifndef STREAM_KEYS_H
#define STREAM_KEYS_H

int stream_keys_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  
  ulfius_set_json_response(response, 200, data->stream_keys);
  return U_OK;
}

int stream_keys_name_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;

  const char *user_name = u_map_get(request->map_url, "name");

  ulfius_set_json_response(response, 200, json_object_get(data->stream_keys, user_name));

  return U_OK;
}

int stream_keys_POST(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;

  json_object_update_existing(data->stream_keys, request->json_body);

  ulfius_set_json_response(response, 200, data->stream_keys);

  return U_OK;
}

#endif /* STREAM_KEYS_H */
