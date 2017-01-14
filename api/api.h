#ifndef API_H
#define API_H

#include "endpoints.h"

int auth_function(const struct _u_request *request,
		  struct _u_response *response,
		  void *user_data)
{
  obs_server_data_t *data = (obs_server_data_t *)user_data;
  const char *auth_secret = u_map_get_case(request->map_header, "Client-ID");

  if(auth_secret && strcmp(auth_secret, data->secret_key) == 0)
      return U_OK;

  return U_ERROR_UNAUTHORIZED;
}
		  

static void load_api_endpoints(struct _u_instance *instance, void *user_data)
{
  unsigned int num_endpoints = sizeof(api_endpoints) / sizeof(api_endpoints[0]);
  int i;

  for(i = 0; i < num_endpoints; ++i) {
    api_endpoints[i].user_data = user_data;
    ulfius_add_endpoint(instance, &api_endpoints[i]);
  }

  ulfius_set_default_auth_function(instance, auth_function, user_data, NULL);

  printf("loaded %d endpoints\n", num_endpoints);
}

#endif
