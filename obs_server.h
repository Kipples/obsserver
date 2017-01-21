#ifndef OBS_SERVER_H
#define OBS_SERVER_H

#include <obs/obs.h>

typedef struct obs_scene_list_t obs_scene_list_t;

struct obs_scene_list_t {
  obs_scene_t *scene;
  obs_scene_list_t *next, *prev;
};

typedef struct obs_server_data_t {
  json_t *layout;
  json_t *stream_keys;
  json_t *name_plate_sources;
  char *secret_key;
  json_t *global;
  obs_scene_list_t *scene_head, *scene_tail;
} obs_server_data_t;


obs_source_t *create_stream_key_source(const char *stream_key)
{
  char str[256];
  
  obs_source_t *video_source = obs_source_create("ffmpeg_source", stream_key, NULL, NULL);
  obs_data_t *video_settings = obs_source_get_settings(video_source);

  obs_data_set_bool(video_settings, "is_local_file", false);
  obs_data_set_bool(video_settings, "restart_on_activate", false);
  sprintf(str, "rtmp://localhost/live/%s", stream_key);
  obs_data_set_string(video_settings, "input", str);

  obs_source_set_muted(video_source, true);
  obs_source_update(video_source, video_settings);

  return video_source;
}

/* void create_stream_key_source(const char *stream_key) */
/* { */
/*     char str[256]; */

/*     obs_data_t *settings = obs_data_create(); */
/*     obs_data_array_t *arr = obs_data_array_create(); */
/*     obs_data_t *d = obs_data_create(); */
/*     sprintf(str, "rtmp://localhost/live/%s", stream_key); */
/*     obs_data_set_string(d, "value", str); */
/*     obs_data_array_push_back(arr, d); */
/*     obs_data_set_array(settings, "playlist", arr); */
/*     obs_data_set_string(settings, "playback_behavior", "always_play"); */

/*     obs_source_t *vlc_source = obs_source_create("vlc_source", stream_key, settings, NULL); */

/*     obs_source_set_muted(vlc_source, true); */
/*     obs_source_update(vlc_source, settings); */
/* } */

void init_obs_server_data(obs_server_data_t *data)
{
  json_error_t error;
  
  data->global = json_load_file("./global.json", 0, &error);

  const char *secret_key = json_string_value(json_object_get(data->global, "secret_key"));

  data->secret_key = malloc(strlen(secret_key));
  data->secret_key = strcpy(data->secret_key, secret_key);

  data->name_plate_sources = json_array();
  data->stream_keys = json_load_file("./stream_keys.json", 0, NULL);

  data->layout = json_object();

  json_object_set_new(data->layout, "num_streams", json_integer(4));

  json_t *user_list = json_array();

  const char *key;
  json_t *value;

  json_object_foreach(data->stream_keys, key, value) {
    // create source
    create_stream_key_source(json_string_value(value));

    // add user name to user_list
    json_array_append_new(user_list, json_string(key));

    // create a user object for layout
    json_t *user = json_object();
    json_object_set_new(user, "muted", json_boolean(true));

    json_object_set_new(data->layout, key, user);

  }

  json_object_set_new(data->layout, "active_user_list", user_list);
  json_object_set_new(data->layout, "inactive_user_list", json_array());
  
  data->scene_head = data->scene_tail = NULL;
}

void obs_server_add_scene_node(obs_server_data_t *data, obs_scene_t *scene)
{
  obs_scene_list_t *scene_list_node = malloc(sizeof(obs_scene_list_t));
  scene_list_node->prev = NULL;
  scene_list_node->next = NULL;
  scene_list_node->scene = scene;

  if(data->scene_head == NULL) {
    data->scene_head = scene_list_node;
    data->scene_tail = scene_list_node;
  } else {
    data->scene_tail->next = scene_list_node;
    scene_list_node->prev = data->scene_tail;
    data->scene_tail = scene_list_node;
  }

}

obs_scene_t *obs_server_add_scene(obs_server_data_t *data, const char *name)
{
  obs_scene_t *scene = obs_scene_create(name);

  obs_server_add_scene_node(data, scene);

  return scene;
}

obs_scene_t *obs_server_duplicate_scene(obs_server_data_t *data, obs_scene_t *scene, const char *name)
{
  obs_scene_t *duplicate_scene = obs_scene_duplicate(scene, name, OBS_SCENE_DUP_REFS);

  obs_server_add_scene_node(data, duplicate_scene);

  return duplicate_scene;
}

#define foreach_scene_list(scene_list, data) for(obs_scene_list_t *scene_list = data->scene_head; scene_list != NULL; scene_list = scene_list->next)

obs_scene_t *obs_server_find_scene_by_name(obs_server_data_t *data, const char *name)
{
  foreach_scene_list(scene_list, data) {
    obs_source_t *scene_source = obs_scene_get_source(scene_list->scene);
    const char *scene_name = obs_source_get_name(scene_source);
    if(strcmp(scene_name, name) == 0) return scene_list->scene;
  }
  
  return NULL;
}


void destroy_obs_server_data(obs_server_data_t *data)
{
  // TODO:
}

#endif
