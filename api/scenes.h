#ifndef API_SCENES_H
#define API_SCENES_H

#include "serialize.h"
#include "../obs_server.h"

int scenes_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();
  json_t *array = json_array();

  foreach_scene_list(scene_list, data) {
    obs_source_t *scene_source = obs_scene_get_source(scene_list->scene);
    const char *scene_name = obs_source_get_name(scene_source);
    if(obs_source_active(scene_source)) {
      //scene is active
      json_object_set_new(json, "active_scene", json_string(scene_name));
    }
    json_array_append_new(array, json_string(scene_name));
  }
  
  json_object_set_new(json, "scenes", array);

  
  ulfius_set_json_response(response, 200, json);

  json_decref(json);
  return U_OK;
}

int scenes_name_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *scene_name = u_map_get(request->map_url, "name");
  obs_scene_t *scene = obs_server_find_scene_by_name(data, scene_name);

  obs_scene_to_json(json, scene, "scene");
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int scenes_name_active_PUT(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  const char *scene_name = u_map_get(request->map_url, "name");
  obs_scene_t *scene = obs_server_find_scene_by_name(data, scene_name);

  obs_set_output_source(0, obs_scene_get_source(scene));
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int scenes_name_POST(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  json_t *json = json_object();

  int rc = 200;

  const char *scene_name = u_map_get(request->map_url, "name");
  obs_scene_t *scene = obs_server_find_scene_by_name(data, scene_name);

  if(request->json_has_error) {
    json_error_to_json(json, request->json_error);
    rc = 400;
  } else {
    json_update_obs_scene(scene, request->json_body);
    obs_scene_to_json(json, scene, "scene");
  }
  
  ulfius_set_json_response(response, rc, json);
  json_decref(json);
  return U_OK;
}

#endif
