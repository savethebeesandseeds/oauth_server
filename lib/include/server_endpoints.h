#ifndef __SERVER_ENDPOINTS
#define __SERVER_ENDPOINTS
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "util_piaabo.h"
#include "server_piaabo.h"
#include "requests_piaabo.h"

#define FAVICON "resources/waacamaya_q1.png"
#define HOME_HTML "resources/home.html"
#define LOGGING_HTML "resources/logging.html"
#define DEFAULT_HTML "resources/default.html"
#define OAUTH_LANDING_HTML "resources/oauth_redirect.html"

#define OAUTH2_SERVICE "https://www.oauth.com/playground/auth-dialog.html"

#define HTML_BUFFER_SIZE (1<<16)
/* favicon.ico */
static enum MHD_Result favicon_route(routing_data_t *route_data){
  return quick_file_response(route_data->connection,
    open(FAVICON, O_RDONLY), "image/png");
}
/* default route */
static enum MHD_Result default_route(routing_data_t *route_data){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* initialize a template_map */
  template_map_queue_t *template_map=initialize_template_map();
  /* list the template arguments */
  add_template_map(template_map, "HTTP_METHOD", route_data->method);
  add_template_map(template_map, "HTTP_VERSION", route_data->version);
  add_template_map(template_map, "URL_ROUTE", route_data->url);
  /* load and replace html template arguments */
  load_html_template(DEFAULT_HTML, template_map, html_buffer);
  /* free the template_map */
  destroy_template_map(template_map);
  /* create the response variable */
  server_response_t *response=create_html_response(route_data->connection, html_buffer);
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* make the response */
  return respond_with_html(response);
}
/* Ask for Authentication */
static enum MHD_Result ask_authentication(routing_data_t *route_data){
  /* render HTML response (ask for auth) */
  return quick_html_response(route_data->connection, 
    "<html><body>Request requires a valid SSL/TLS certificate!</body></html>");
}
/* logging route */
static enum MHD_Result logging_route(routing_data_t *route_data){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  // char oauth_service_buffer[HTML_BUFFER_SIZE]="\0";
  // char *oauth_service_redirect_url = oauth_service_buffer;
  /* initialize a template_map */
  template_map_queue_t *template_map=initialize_template_map();
  /* encode the oauth2 redirection route */
  // params_queue_t *params=queue_fabric();
  // add_to_params_queue(params, "response_type", "code");
  // add_to_params_queue(params, "client_id", "7CCUISrBbfkLdJ46p7RohFKo");
  // add_to_params_queue(params, "redirect_uri", "http://localhost:8888/");
  // add_to_params_queue(params, "scope", "photo+offline_access");
  // add_to_params_queue(params, "state", "du9JpZqN6LKO_gX4");
  // encode_get_params_minimal(params, &oauth_service_redirect_url);
  // queue_destructor(params);
  // log_warn("oauth_service_redirect_url: %s\n",oauth_service_redirect_url);
  /* list the template arguments */
  add_template_map(template_map, "OAUTH_URL", "wakaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  /* load and replace html template arguments */
  load_html_template(LOGGING_HTML, template_map, html_buffer);
  /* free the template_map */
  destroy_template_map(template_map);
  /* create the response variable */
  server_response_t *response=create_html_response(route_data->connection, html_buffer);
  /* add headers */
  add_response_header(response, "Link", "logging");
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* make the response */
  return respond_with_html(response);
}
/* oauth2_landing */
static enum MHD_Result oauth2_landing(routing_data_t *route_data){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* load and replace html template arguments */
  load_html_template(OAUTH_LANDING_HTML, NULL, html_buffer);
  /* create the response variable */
  server_response_t *response=create_html_response(route_data->connection, html_buffer);
  /* add headers */
  set_session_cookie(response);
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* make the response */
  return respond_with_html(response);
}
/* home */
static enum MHD_Result home(routing_data_t *route_data){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* load and replace html template arguments */
  load_html_template(HOME_HTML, NULL, html_buffer);
  /* respond with html */
  return quick_html_response(route_data->connection, html_buffer);
}

/* selects the appropriate route when the conexion is secure */
static route_pointer* secure_route_coordinator(routing_data_t *route_data){
  if(!strcmp(route_data->method,"GET") && !strcmp(route_data->url,"/favicon.ico")){
    return favicon_route;
  } else if(!strcmp(route_data->url,"/home")){
    return home;
  } else {
    return default_route;
  }
}
/* selects the appropriate route when the conexion is insecure */
static route_pointer* route_coordinator(routing_data_t *route_data){
  if(!strcmp(route_data->method,"GET") && !strcmp(route_data->url,"/favicon.ico")){
    return favicon_route;
  } else if(!strcmp(route_data->url,"/logging")){
    return logging_route;
  } else if(!strcmp(route_data->url,"/oauth2_landing")){
    return oauth2_landing;
  } else {
    return default_route;
  }
}
#endif