#ifndef SOURCES_H
#define SOURCES_H

bool sources_GET_enum(void *user_data, obs_source_t *source)
{
  json_t *sources_arr = (json_t *)user_data;

  json_array_append_new(sources_arr, json_string(obs_source_get_name(source)));

  return true;
}

int sources_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  json_t *sources_arr = json_array();

  obs_enum_sources(sources_GET_enum, (void *)sources_arr);
  
  json_object_set_new(json, "sources", sources_arr);
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int sources_name_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *source_name = u_map_get(request->map_url, "name");
  obs_source_t *source = obs_get_source_by_name(source_name);

  obs_source_to_json(json, source, "source");
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int sources_name_POST(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  int rc = 200;

  const char *source_name = u_map_get(request->map_url, "name");
  obs_source_t *source = obs_get_source_by_name(source_name);

  const char *key;
  json_t *value;

  if(request->json_has_error) {
    json_error_to_json(json, request->json_error);
    rc = 400;
  } else {
    json_update_obs_source(source, json_object_get(request->json_body, "source"));
    obs_source_to_json(json, source, "source");
  }
  
  ulfius_set_json_response(response, rc, json);
  json_decref(json);
  return U_OK;
}



#endif /* SOURCES_H */
