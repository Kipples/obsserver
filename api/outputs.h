#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "../obs_server.h"
#include "serialize.h"

bool outputs_GET_enum(void *user_data, obs_output_t *output)
{
  json_t *json = (json_t *)user_data;

  json_array_append_new(json_object_get(json, "outputs"),
			json_string(obs_output_get_name(output)));
  
  return true;
}

int outputs_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  json_t *arr = json_array();

  json_object_set_new(json, "outputs", arr);
  
  obs_enum_outputs(outputs_GET_enum, json);
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int outputs_name_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *output_name = u_map_get(request->map_url, "name");
  obs_output_t *output = obs_get_output_by_name(output_name);

  obs_output_to_json(json, output, "output");
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int outputs_name_active_PUT(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *output_name = u_map_get(request->map_url, "name");
  obs_output_t *output = obs_get_output_by_name(output_name);

  if(!obs_output_active(output)) {
    bool success = obs_output_start(output);
    json_object_set_new(json, "start", json_boolean(success));
  }
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int outputs_name_active_DELETE(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *output_name = u_map_get(request->map_url, "name");
  obs_output_t *output = obs_get_output_by_name(output_name);

  obs_output_stop(output);

  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

#endif /* OUTPUTS_H */
