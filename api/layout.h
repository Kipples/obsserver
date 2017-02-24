#ifndef LAYOUT_H
#define LAYOUT_H

#include "../obs_server.h"

int layout_GET(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  
  ulfius_set_json_response(response, 200, data->layout);

  return U_OK;
}

int layout_POST(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;

  json_object_update_existing(data->layout, request->json_body);

  ulfius_set_json_response(response, 200, request->json_body);

  return U_OK;
}

int layout_reflow_PUT(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t*)user_data;
  
  int num_streams = json_integer_value(json_object_get(data->layout, "num_streams"));

  json_t *arr = json_object_get(data->layout, "inactive_user_list");
  char str[32];

  int i;
  json_t *value;

  json_array_foreach(arr, i, value) {
    //cleanup inactive users

    const char *user_name = json_string_value(value);
    const char * stream_key = json_string_value(json_object_get(data->stream_keys, user_name));
    
    json_t *user = json_object_get(data->layout, user_name);

    json_object_set_new(user, "muted", json_boolean(true));

    json_object_set(data->layout, user_name, user);

    obs_source_t *source = obs_get_source_by_name(stream_key);
    obs_source_set_muted(source, true);

    obs_sceneitem_t *item;
    foreach_scene_list(scene_list, data) {
      item = obs_scene_find_source(scene_list->scene, stream_key);
      if(item) break;
    }
    
    obs_sceneitem_remove(item);
  }

  arr = data->name_plate_sources;
  
  json_array_foreach(arr, i, value) {
    const char *s = json_string_value(value);

    obs_source_t *source = obs_get_source_by_name(s);

    obs_data_t *text_source_data = obs_source_get_settings(source);

    obs_data_set_string(text_source_data, "text", " ");

    obs_source_update(source, text_source_data);
  }

  arr = json_object_get(data->layout, "active_user_list");


  json_array_foreach(arr, i, value) {

    const char *user_name = json_string_value(value);
    json_t *user = json_object_get(data->layout, user_name);

    int scene_offset = i / num_streams;
    int location_offset = i % num_streams;

    sprintf(str, "%dview_%d", num_streams, scene_offset);

    obs_scene_t *scene = obs_server_find_scene_by_name(data, str);

    if(!scene) break;

    const char * stream_key = json_string_value(json_object_get(data->stream_keys, user_name));
    obs_sceneitem_t *item;
    foreach_scene_list(scene_list, data) {
      item = obs_scene_find_source(scene_list->scene, stream_key);
      if(item) break;
    }
    
    obs_sceneitem_remove(item);

    obs_source_t *source = obs_get_source_by_name(stream_key);

    bool is_muted = json_boolean_value(json_object_get(user, "muted"));

    obs_source_set_muted(source, is_muted);
    
    item = obs_scene_add(scene, source);

    struct vec2 pos;

    struct vec2 bounds;

    source = obs_scene_get_source(scene);

    obs_data_t *obs_data = obs_source_get_settings(source);

    obs_data_t *virtual_locations = obs_data_get_obj(obs_data, "virtual_locations");

    sprintf(str, "scene_loc_%d", location_offset);
    obs_data_t *scene_location = obs_data_get_obj(virtual_locations, str);

    obs_data_get_vec2(scene_location, "pos", &pos);
    
    obs_data_get_vec2(scene_location, "bounds", &bounds);

    obs_sceneitem_set_pos(item, &pos);

    obs_sceneitem_set_bounds_type(item, OBS_BOUNDS_SCALE_INNER);
    obs_sceneitem_set_bounds(item, &bounds);

    obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);
    obs_sceneitem_set_visible(item, true);

    sprintf(str, "%dview_%dscene_%dtext", num_streams, scene_offset, location_offset);
    source = obs_get_source_by_name(str);

    obs_data_t *text_source_data = obs_source_get_settings(source);

    obs_data_set_string(text_source_data, "text", user_name);

    obs_source_update(source, text_source_data);

    sprintf(str, "%dview_%dscene_%daudio", num_streams, scene_offset, location_offset);

    obs_sceneitem_t *audio = obs_scene_find_source(scene, str);

    obs_sceneitem_set_visible(audio, !is_muted);
    
  }
  
  ulfius_set_json_response(response, 200, data->layout);

  return U_OK;
}


#endif /* LAYOUT_H */
