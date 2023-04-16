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

#define FAVICON "resources/waacamaya_q1.png"
#define HOME_HTML "resources/home.html"
#define LOGGING_HTML "resources/logging.html"
#define OAUTH_LANDING_HTML "resources/oauth_redirect.html"

#define HTML_BUFFER_SIZE (1<<16)
/* favicon.ico */
static enum MHD_Result favicon_route(
  void *dh_cls, struct MHD_Connection *connection){
  return quick_file_response(connection,
    open(FAVICON, O_RDONLY), "image/png");
}
/* default route */
static enum MHD_Result default_route(
  void *dh_cls, struct MHD_Connection *connection){
  /* render HTML response (default) */
  return quick_html_response(connection, 
    "<html><body>Default route!</body></html>");
}
/* Ask for Authentication */
static enum MHD_Result ask_authentication(
  void *dh_cls, struct MHD_Connection *connection){
  /* render HTML response (ask for auth) */
  return quick_html_response(connection, 
    "<html><body>Request requires a valid SSL/TLS certificate!</body></html>");
}
/* logging route */
static enum MHD_Result logging_route(
  void *dh_cls, struct MHD_Connection *connection){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* initialize a template_map */
  template_map_queue_t *template_map=initialize_template_map();
  /* list the template arguments */
  add_template_map(template_map, "OAUTH_URL", "waka");
  /* load and replace html template arguments */
  load_html_template(LOGGING_HTML, template_map, html_buffer);
  /* free the template_map */
  destroy_template_map(template_map);
  /* create the response variable */
  server_response_t *response=create_html_response(connection, html_buffer);
  /* add headers */
  add_header(response, "Link", "logging");
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* make the response */
  return respond_with_html(response);
}
/* oauth2_landing */
static enum MHD_Result oauth2_landing(
  void *dh_cls, struct MHD_Connection *connection){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* load and replace html template arguments */
  load_html_template(OAUTH_LANDING_HTML, NULL, html_buffer);
  /* create the response variable */
  server_response_t *response=create_html_response(connection, html_buffer);
  /* add headers */
  set_session_cookie(response);
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* make the response */
  return respond_with_html(response);
}
/* home */
static enum MHD_Result home(
  void *dh_cls,
  struct MHD_Connection *connection){
  char html_buffer[HTML_BUFFER_SIZE]="\0";
  /* load and replace html template arguments */
  load_html_template(HOME_HTML, NULL, html_buffer);
  /* respond with html */
  return quick_html_response(connection, html_buffer);
}

/* selects the appropriate route when the conexion is secure */
static route_pointer* secure_route_coordinator(
  const char *url,
  const char *method, 
  const char *version){
  if(!strcmp(method,"GET") && !strcmp(url,"/favicon.ico")){
    return favicon_route;
  } else if(!strcmp(url,"/home")){
    return home;
  } else {
    return default_route;
  }
}
/* selects the appropriate route when the conexion is insecure */
static route_pointer* route_coordinator(
  const char *url,
  const char *method, 
  const char *version){
  if(!strcmp(method,"GET") && !strcmp(url,"/favicon.ico")){
    return favicon_route;
  } else if(!strcmp(url,"/logging")){
    return logging_route;
  } else if(!strcmp(url,"/oauth2_landing")){
    return oauth2_landing;
  } else {
    return default_route;
  }
}
#endif