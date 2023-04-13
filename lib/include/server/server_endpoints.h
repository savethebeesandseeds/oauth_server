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
#define LOGGING_HTML "resources/logging.html"

/* favicon.ico */
static enum MHD_Result favicon_route(
  void *dh_cls, struct MHD_Connection *connection){
  return respond_with_file(
    open(FAVICON, O_RDONLY), "image/png", 
    connection, MHD_HTTP_OK, NULL, NULL);
}

/* default route */
static enum MHD_Result default_route(
  void *dh_cls,
  struct MHD_Connection *connection){
  /* render HTML response (default) */
  return respond_with_html(
    "<html><body>Default route!</body></html>", 
    connection, MHD_HTTP_OK, NULL, NULL);
}
/* Ask for Authentication */
static enum MHD_Result ask_for_tls_authentication(
  struct MHD_Connection *connection){
  return respond_with_html(
      "<html><body>Request requires a valid SSL/TLS certificate!</body></html>", 
      connection, MHD_HTTP_UNAUTHORIZED, NULL, NULL);
}
/* logging route */
static enum MHD_Result logging_route(
  void *dh_cls,
  struct MHD_Connection *connection){
  char output[1<<16]="\0"; // #FIXME do we really need a buffer?
  // create an array of Person structs
  HTML_TemplateKeyValue templateKeyValues[MAX_TEMPLATE_REPLACEMENTS] = {
    {"OAUTH_URL", "waka1"}
  };
  /* render HTML response (logging) */
  load_html_template(LOGGING_HTML, templateKeyValues, output);
  /* respond with html */
  return respond_with_html(
    "<html><body>Default route!</body></html>", 
    connection, MHD_HTTP_OK, NULL, NULL);
}

/* oauth2_redirect */
static enum MHD_Result oauth2_redirect(
  void *dh_cls,
  struct MHD_Connection *connection){
  /* Respong with HTML */
  return respond_with_html(
    "<html><body>Hello, oauth!</body></html>", 
    connection, MHD_HTTP_OK, NULL, NULL);
}

/* selects the appropriate route when the conexion is secure */
static route_pointer* secure_route_coordinator(
  const char *url,
  const char *method, 
  const char *version){
  if(!strcmp(method,"GET") && !strcmp(url,"/favicon.ico")){
    return favicon_route;
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
  } else if(!strcmp(url,"/oauth2_redirect")){
    return oauth2_redirect;
  } else {
    return default_route;
  }
}
#endif