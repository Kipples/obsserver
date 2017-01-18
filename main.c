#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <obs/obs.h>
#include <ulfius.h>
#include "obs_server.h"
#include "api/api.h"

#define PORT 9009

static void render_window(void *data, uint32_t cx, uint32_t cy)
{
  obs_render_main_view();
}

static void load_required_modules(void *param, const struct obs_module_info *info)
{
  obs_module_t *module;

  int code = obs_open_module(&module, info->bin_path, info->data_path);

  const char *module_file_name = obs_get_module_file_name(module);
  if(module_file_name) {
    // when module file name is not "frontend-tools.so"
    if(strcmp(module_file_name, "frontend-tools.so")) // this breaks when not using QT
      // initialize the module
      obs_init_module(module);
  }
}

static struct vec2 scene_4_stream_positions[] = {{ .x = 19.0, .y = 19.0},
						  { .x = 489.0, .y = 19.0},
						  { .x = 19.0, .y = 370.0},
						  { .x = 489.0, .y = 370.0}};
static struct vec2 scene_4_stream_bounds = { .x = 441.0, .y = 326.0 };

static struct vec2 scene_4_name_plate_positions [] = {{ .x = 995.0, .y = 16.0},
						      { .x = 995.0, .y = 92.0},
						      { .x = 995.0, .y = 168.0 },
						      { .x = 995.0, .y = 244.0 }};

static struct vec2 scene_4_timer_pos = { .x = 973.0, .y = 338.0 };

static struct vec2 scene_3_stream_positions[] = {{ .x = 19.0, .y = 19.0},
						 { .x = 489.0, .y = 19.0},
						 { .x = 19.0, .y = 370.0}};
static struct vec2 scene_3_stream_bounds = { .x = 441.0, .y = 326.0 };

static struct vec2 scene_3_name_plate_positions [] = {{ .x = 650.0, .y = 410.0},
						      { .x = 705.0, .y = 500.0},
						      { .x = 760.0, .y = 590.0 }};

static struct vec2 scene_3_timer_pos = { .x = 968.0, .y = 20.0 };

static struct vec2 scene_2_stream_positions[] = {{ .x = 0.0, .y = 19.0},
						 { .x = 642.0, .y = 19.0}};
static struct vec2 scene_2_stream_bounds = { .x = 639.0, .y = 480.0 };

static struct vec2 scene_2_name_plate_positions [] = {{ .x = 110.0, .y = 530.0},
						      { .x = 730.0, .y = 530.0 }};

static struct vec2 scene_2_timer_pos = { .x = 542.0, .y = 594.0 };

static void create_scene_name_plates(obs_server_data_t *data,
				     obs_scene_t *scene, int num_streams, int scene_num,
				     struct vec2 name_plate_positions[])
{
  char str[32];

  for(int i = 0; i < num_streams; i++) {
    sprintf(str, "%dview_%dscene_%dtext", num_streams, scene_num, i);
    obs_source_t *text_source = obs_source_create("text_ft2_source", str, NULL, NULL);
    obs_sceneitem_t *item = obs_scene_add(scene, text_source);
    obs_sceneitem_set_pos(item, &(name_plate_positions[i]));
    json_array_append_new(data->name_plate_sources, json_string(str));
  }
}

static void create_player_scenes(obs_server_data_t *data, int num_scenes, int num_streams,
				 struct vec2 positions[], struct vec2 *bounds,
				 struct vec2 name_plate_postitions [],
				 struct vec2 *timer_pos,
				 obs_source_t *timer_source,
				 obs_source_t *image_source)
{
  int i;
  char str[32];
  
  sprintf(str, "%dview_0", num_streams);
  obs_scene_t *scene = obs_server_add_scene(data, str);

  obs_scene_add(scene, image_source);
  obs_sceneitem_t *item = obs_scene_add(scene, timer_source);

  obs_sceneitem_set_pos(item, timer_pos);
	
  obs_source_t *source = obs_scene_get_source(scene);

  obs_data_t *source_data = obs_source_get_settings(source);

  obs_data_t *scene_meta_data = obs_data_create();
  
  obs_data_t *virtual_locations = obs_data_create();

  for(i = 0; i < num_streams; i++) {
    obs_data_t *scene_obj = obs_data_create();
    sprintf(str, "scene_loc_%d", i);
    obs_data_set_string(scene_obj, "id", str);
    obs_data_set_vec2(scene_obj, "pos", &(positions[i]));
    obs_data_set_vec2(scene_obj, "bounds", bounds);
    obs_data_set_obj(virtual_locations, str, scene_obj);
    obs_data_release(scene_obj);
  }

  obs_data_set_obj(scene_meta_data, "virtual_locations", virtual_locations);
  obs_data_set_int(scene_meta_data, "num_streams", num_streams);

  obs_data_apply(source_data, scene_meta_data);

  obs_source_update(source, source_data);

  for(i = 1; i < num_scenes; i++) {
    sprintf(str, "%dview_%d", num_streams, i);
    obs_scene_t *s = obs_server_duplicate_scene(data, scene, str);
    source = obs_scene_get_source(s);
    source_data = obs_source_get_settings(source);
    obs_data_apply(source_data, scene_meta_data);
    obs_source_update(source, source_data);
    create_scene_name_plates(data, s, num_streams, i, name_plate_postitions);
  }

  create_scene_name_plates(data, scene, num_streams, 0, name_plate_postitions);

  obs_data_release(virtual_locations);
  obs_data_release(scene_meta_data);
}

obs_source_t *create_image_source(const char *name, const char *image_file)
{
  obs_source_t *image_source = obs_source_create("image_source", name, NULL, NULL);
  obs_data_t *image_settings = obs_source_get_settings(image_source);
  obs_data_set_string(image_settings, "file", image_file);

  obs_source_update(image_source, image_settings);
  obs_data_release(image_settings);

  return image_source;
}

obs_source_t *create_window_capture(const char *window_title)
{
  char str[64];

  strcpy(str, "\r\n");
  strcat(str, window_title);
  strcat(str, "\r\n");
  
  obs_source_t *window_source = obs_source_create("xcomposite_input", window_title, NULL, NULL);
  obs_data_t *window_settings = obs_source_get_settings(window_source);
    
  obs_data_set_string(window_settings, "capture_window", str);
  obs_data_set_bool(window_settings, "exclude_alpha", true);

  obs_source_update(window_source, window_settings);
  obs_data_release(window_settings);

  return window_source;
}

int main(int argc, char **argv)
{

  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *win = SDL_CreateWindow("Hi",
				     0,
				     0,
				     1280,
				     720,
				     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext gl = SDL_GL_CreateContext(win);

  SDL_GL_MakeCurrent(win, gl);

  obs_startup("en-US", NULL, NULL);

  struct obs_video_info ovi;
  ovi.graphics_module = "libobs-opengl";
  ovi.fps_num = 60;
  ovi.fps_den = 1;
  
  ovi.base_width = 1280;
  ovi.output_width = 1280;
  
  ovi.base_height = 720;
  ovi.output_height = 720;
  
  ovi.output_format = VIDEO_FORMAT_NV12;
  ovi.adapter = 0;

  obs_reset_video(&ovi);

  struct obs_audio_info oai;

  oai.samples_per_sec = 44100;
  oai.speakers = SPEAKERS_MONO;

  obs_reset_audio(&oai);

  obs_find_modules(load_required_modules, NULL);

  // Source

  obs_server_data_t server_data;

  init_obs_server_data(&server_data);
  
  obs_source_t *view4_image_source = create_image_source("4viewimage", "images/dkclayout_4view.png");
  obs_source_t *view3_image_source = create_image_source("3viewimage", "images/dkclayout_3view.png");
  obs_source_t *view2_image_source = create_image_source("2viewimage", "images/dkclayout_2view.png");

  obs_source_t *timer_source = create_window_capture("Timer");

  create_player_scenes(&server_data, 9, 4, scene_4_stream_positions, &scene_4_stream_bounds, scene_4_name_plate_positions, &scene_4_timer_pos, timer_source, view4_image_source);
  create_player_scenes(&server_data, 9, 3, scene_3_stream_positions, &scene_3_stream_bounds, scene_3_name_plate_positions, &scene_3_timer_pos, timer_source, view3_image_source);
  create_player_scenes(&server_data, 9, 2, scene_2_stream_positions, &scene_2_stream_bounds, scene_2_name_plate_positions, &scene_2_timer_pos, timer_source, view2_image_source);

  obs_source_release(view4_image_source);
  obs_source_release(view3_image_source);
  obs_source_release(view2_image_source);
  obs_source_release(timer_source);

  obs_source_t *audio_source = obs_source_create("pulse_output_capture", "audio", NULL, NULL);

  obs_scene_t *scene = obs_server_find_scene_by_name(&server_data, "4view_0");
  
  obs_set_output_source(0, obs_scene_get_source(scene));
  obs_set_output_source(1, audio_source);
  
  obs_scene_release(scene);
  obs_source_release(audio_source);

  obs_encoder_t *h264_encoder = obs_video_encoder_create("obs_x264", "simple_h264_stream", NULL, NULL);
  obs_encoder_t *aac_encoder = obs_audio_encoder_create("ffmpeg_aac", "simple_aac_stream", NULL, 0, NULL);

  obs_encoder_set_video(h264_encoder, obs_get_video());
  obs_encoder_set_audio(aac_encoder, obs_get_audio());

  obs_output_t *output = obs_output_create("rtmp_output", "live_stream", NULL, NULL);

  obs_service_t *service = obs_service_create("rtmp_custom", "nginx_rtmp", NULL, NULL);

  obs_data_t *service_settings = obs_service_get_settings(service);
  obs_data_set_string(service_settings, "server", "rtmp://localhost/twitch/");
  obs_data_set_string(service_settings, "key", "dkcspeedruns"); //Doesn't really matter what we put here
  obs_service_update(service, service_settings);
  obs_data_release(service_settings);

  obs_output_set_video_encoder(output, h264_encoder);
  obs_output_set_audio_encoder(output, aac_encoder, 0);
  
  obs_output_set_service(output, service);

  /* obs_output_t *output = obs_output_create("ffmpeg_output", "live_stream", NULL, NULL); */
  /* obs_output_set_preferred_size(output, 1280, 720); */

  /* obs_data_t *output_settings = obs_output_get_settings(output); */

  /* obs_data_set_string(output_settings, "url", "rtmp://localhost:1935/twitch/dkcspeedruns"); */
  /* obs_data_set_int(output_settings, "video_bitrate", 3500); */
  /* obs_data_set_int(output_settings, "audio_bitrate", 160); */
  
  /* obs_output_update(output, output_settings); */

  struct gs_init_data info = {};

  int w,h;
  
  SDL_GL_GetDrawableSize(win, &w, &h);

  info.cx = w;
  info.cy = h;
  info.format = GS_RGBA;
  info.zsformat = GS_ZS_NONE;

  SDL_SysWMinfo sdl_wm_info;

  SDL_VERSION(&sdl_wm_info.version);

  SDL_GetWindowWMInfo(win, &sdl_wm_info);

  switch(sdl_wm_info.subsystem) {
  case SDL_SYSWM_X11:
    info.window.display = sdl_wm_info.info.x11.display;
    info.window.id = sdl_wm_info.info.x11.window;
    break;
  default:
    break;    
  }

  obs_display_t *dis = obs_display_create(&info);

  obs_display_add_draw_callback(dis, render_window, NULL);

  SDL_Event e;
  int quit = 0;

  struct _u_instance instance;

  ulfius_init_instance(&instance, PORT, NULL);

  load_api_endpoints(&instance, &server_data);

  ulfius_start_framework(&instance);

  while(!quit) {
    while(SDL_PollEvent(&e)) {
      switch(e.type) {
      case SDL_QUIT:
	quit = 1;
	break;
      case SDL_WINDOWEVENT:
	switch(e.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
	  obs_display_resize(dis, e.window.data1, e.window.data2);
	  break;
	}
	break;
      }
    }
  }

  destroy_obs_server_data(&server_data);

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  obs_shutdown();

  return 0;
}
