#ifndef SERIALIZE_H
#define SERIALIZE_H

#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC 1000000
#endif

static void json_error_to_json(json_t *json, json_error_t *error)
{
  json_object_set_new(json, "error", json_string(error->text));
}

static void obs_data_to_json(json_t *json, obs_data_t *data, const char *key)
{
  json_t *data_json = json_loads(obs_data_get_json(data), 0, NULL);
  json_object_set_new(json, key, data_json);
}

static void json_data_to_obs_data(json_t *json, obs_data_t *target)
{
  char *json_string = json_dumps(json, 0);

  obs_data_t *data = obs_data_create_from_json(json_string);

  obs_data_apply(target, data);

  free(json_string);
  obs_data_release(data);
}

static void obs_source_to_json(json_t *json, obs_source_t *source, const char *key)
{
  json_t *source_obj = json_object();

  json_object_set_new(source_obj, "name", json_string(obs_source_get_name(source)));
  json_object_set_new(source_obj, "id", json_string(obs_source_get_id(source)));
  json_object_set_new(source_obj, "active", json_boolean(obs_source_active(source)));
  json_object_set_new(source_obj, "delay", json_integer(obs_source_get_sync_offset(source) / NSEC_PER_MSEC));
  obs_data_to_json(source_obj, obs_source_get_settings(source), "data");

  json_object_set_new(json, key, source_obj);
}

static void obs_vec2_to_json(json_t *json, struct vec2 *vec, const char *key)
{
  json_t *vec_obj = json_object();

  json_object_set_new(vec_obj, "x", json_real(vec->x));
  json_object_set_new(vec_obj, "y", json_real(vec->y));

  json_object_set_new(json, key, vec_obj);
}

bool scene_item_enum(obs_scene_t *scene, obs_sceneitem_t *item, void *user_data)
{
  json_t *scene_item_arr = (json_t *)user_data;
  json_t *scene_item_obj = json_object();

  obs_source_t *source = obs_sceneitem_get_source(item);
  obs_source_to_json(scene_item_obj, source, "source");

  struct vec2 vec;

  obs_sceneitem_get_pos(item, &vec);
  obs_vec2_to_json(scene_item_obj, &vec, "pos");

  obs_sceneitem_get_scale(item, &vec);
  obs_vec2_to_json(scene_item_obj, &vec, "scale");

  obs_sceneitem_get_bounds(item, &vec);
  obs_vec2_to_json(scene_item_obj, &vec, "bounds");

  json_array_append_new(scene_item_arr, scene_item_obj);

  return true;
}

static void obs_scene_to_json(json_t *json, obs_scene_t *scene, const char *key)
{
  json_t *scene_obj = json_object();

  obs_source_t *source = obs_scene_get_source(scene);

  obs_source_to_json(scene_obj, source, "source");

  json_t *scene_item_arr = json_array();
  
  obs_scene_enum_items(scene, scene_item_enum, (void *) scene_item_arr);

  json_object_set_new(scene_obj, "scene_items", scene_item_arr);
		      
  json_object_set_new(json, key, scene_obj);
}

static void json_update_obs_source(obs_source_t *source, json_t *json)
{
  const char *key;
  json_t *value;

  json_object_foreach(json, key, value) {
    if(strcmp(key, "data") == 0) {
      json_data_to_obs_data(value, obs_source_get_settings(source));
    }
    if(strcmp(key, "delay") == 0) {
      obs_source_set_sync_offset(source, json_integer_value(value) * NSEC_PER_MSEC);
    }
  }
}

static void json_update_obs_scene(obs_scene_t *scene, json_t *json)
{
  const char *key;
  json_t *value;

  json_object_foreach(json_object_get(json, "scene"), key, value) {
    if(strcmp(key, "source") == 0) {
      json_update_obs_source(obs_scene_get_source(scene), value);
    }
  }
}

static void obs_output_to_json(json_t *json, obs_output_t *output, const char *key)
{
  json_t *output_obj = json_object();

  json_object_set_new(output_obj, "active", json_boolean(obs_output_active(output)));

  obs_data_to_json(output_obj, obs_output_get_settings(output), "data");
  
  json_object_set_new(json, key, output_obj);
}

#endif
