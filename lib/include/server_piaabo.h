#ifndef __SERVER_NOAUTH_PIAABO
#define __SERVER_NOAUTH_PIAABO
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <microhttpd.h>

#include "util_piaabo.h"
#include "file_piaabo.h"
#include "queue_piaabo.h"
#include "response_piaabo.h"

#define MHD_RequestTerminationCode_to_string (const char[4][40]) {\
  "TERMINATED_COMPLETED_OK","TERMINATED_WITH_ERROR","TERMINATED_TIMEOUT_REACHED","TERMINATED_DAEMON_SHUTDOWN"}

#define SESSION_COOKIE_SIZE (1<<6)  /* 64: this is the length of the session cookie  */
#define TEMPLATE_MAP_BUFFER_SIZE (1<<16) /* 65536: the maximun length a template replacement can have */

typedef struct {
  void *dh_cls;
  struct MHD_Connection *connection;
  const char *url;
  const char *method;
  const char *version;
  const char *upload_data;
  size_t *upload_data_size;
  void **con_cls;
  gnutls_session_t tls_session;
  gnutls_x509_crt_t client_cert;
  char *distinguished_name;
  char *alt_name;
} routing_data_t;

typedef const char* output_char_function_pointer();
typedef enum MHD_Result route_pointer(routing_data_t *route_data);

typedef __queue_t template_map_queue_t;
typedef struct {char keyWord[TEMPLATE_MAP_BUFFER_SIZE]; char replaceBy[TEMPLATE_MAP_BUFFER_SIZE];} template_map_type_t;

/* initialize routing data */
routing_data_t initialize_routing_data(
  void *dh_cls, struct MHD_Connection *connection, const char *url,
  const char *method, const char *version){
  routing_data_t route_data={};
  route_data.dh_cls=dh_cls;
  route_data.connection=connection;
  route_data.url=url;
  route_data.method=method;
  route_data.version=version;
  route_data.upload_data=NULL;
  route_data.upload_data_size=0;
  route_data.con_cls=NULL;
  route_data.tls_session=NULL;
  route_data.client_cert=NULL;
  route_data.distinguished_name=NULL;
  route_data.alt_name=NULL;
  return route_data;
}
/* access function to process Incomming request */
static enum MHD_Result on_client_connect(
  void *cls, const struct sockaddr *addr, socklen_t addrlen){
  /* incomming request, new request detected */
	log("--- --- --- %sConnection started%s.\n",
    ANSI_COLOR_Green,ANSI_COLOR_RESET);
  /* allocate space for the ip string */
  char ip_str[addrlen]; /* allocate space for address string */
  /* catch and standarize ip address */
  inet_ntop(
    addr->sa_family, 
    addr->sa_family == AF_INET6 
    ? (void*)(&((struct sockaddr_in6 *)addr)->sin6_addr) 
    : (void*)(&((struct sockaddr_in *)addr)->sin_addr), 
    ip_str, addrlen);
  /* log the adrees */
  log("---%s Incomming from ip(%s)%s: %s\n",
    ANSI_COLOR_Bright_Yellow,
    addr->sa_family == AF_INET6 ? "v6" : "v4", 
    ANSI_COLOR_RESET, ip_str);
  /* proceed to the server */
  return MHD_YES;
}
/* finalization function to process request */
void on_request_completed(
  void *dh_cls, 
  struct MHD_Connection *connection,
  void **con_cls,
  enum MHD_RequestTerminationCode toe){
  /* finalizing request */
  log("--- --- --- %sRequest completed%s: %s.\n",
    ANSI_COLOR_Green,ANSI_COLOR_RESET,
    MHD_RequestTerminationCode_to_string[toe]);
}

/* log request headers */
static enum MHD_Result log_connection_value(
  void *cls, enum MHD_ValueKind kind, 
  const char *key, const char *value){
  char str_kind[40];
  switch (kind){
  case MHD_HEADER_KIND:       strcpy(str_kind,"HEADER");   break;
  case MHD_COOKIE_KIND:       strcpy(str_kind,"COOKIE");   break;
  case MHD_POSTDATA_KIND:     strcpy(str_kind,"POSTDATA"); break;
  case MHD_GET_ARGUMENT_KIND: strcpy(str_kind,"GET_ARG");  break;
  case MHD_FOOTER_KIND:       strcpy(str_kind,"FOOTER");   break;
  default:                    strcpy(str_kind,"UNKNOWN");  break;
  }
  /* log key, value pair */
  log("%s:\t%s%s%s: %s\n", str_kind,(char*)cls, key, ANSI_COLOR_RESET, value);
  /* return success */
  return MHD_YES;
}
/* log request information */
static void log_request(routing_data_t *route_data){
  /* log path and version */
  log("--- %s[%s] request%s, for path: %s, using version %s\n", 
    ANSI_COLOR_Bright_Yellow, route_data->method, ANSI_COLOR_RESET, route_data->url, route_data->version);
  /* log tls identity */
  if(route_data->distinguished_name != NULL)
    log("TLS CERT:\tdistinguished_name:%s\n",route_data->distinguished_name);
  if(route_data->alt_name != NULL)
    log("TLS CERT:\talt_name:%s\n",route_data->alt_name);
  /* log request values */
  MHD_get_connection_values(route_data->connection, MHD_HEADER_KIND, &log_connection_value, (void *)&ANSI_COLOR_Red);
  MHD_get_connection_values(route_data->connection, MHD_COOKIE_KIND, &log_connection_value, (void *)&ANSI_COLOR_Magenta);
  MHD_get_connection_values(route_data->connection, MHD_POSTDATA_KIND, &log_connection_value, (void *)&ANSI_COLOR_Blue);
  MHD_get_connection_values(route_data->connection, MHD_GET_ARGUMENT_KIND, &log_connection_value, (void *)&ANSI_COLOR_Green);
  MHD_get_connection_values(route_data->connection, MHD_FOOTER_KIND, &log_connection_value, (void *)&ANSI_COLOR_Bright_Yellow);
}
/* session manager with cookies */
static void make_session_cookie(char *set_value){
  log_warn("Session Cookie needs to be set better than just random.\n");
  /* autiliary variable */
  char raw_value[SESSION_COOKIE_SIZE];
  /* random method to create cookie */
  for (unsigned int i=0;i<sizeof(raw_value);i++)
    raw_value[i]='A' + (rand () % 26); /* bad PRNG! */
  raw_value[64] = '\0';
  snprintf(set_value, 2*SESSION_COOKIE_SIZE, "%s=%s","SESSION", raw_value);
}
/*  */
static inline const char* get_session_cookie(server_response_t *response){
  /* reading cookies */
  return MHD_lookup_connection_value(
      response->mhd_connection, MHD_COOKIE_KIND, "SESSION");
}
/* set session cookie */
static enum MHD_Result set_session_cookie(server_response_t *response){
  /* autiliary variable */
  char buffer[2*SESSION_COOKIE_SIZE];
  /* autiliary variable */
  if(get_session_cookie(response) == NULL){
    log("Setting Session cookie\n");
    /* produce a value for the sessoin cookie */
    make_session_cookie(buffer);
    /* setting cookie */
    return MHD_add_response_header(
      response->mhd_response, MHD_HTTP_HEADER_SET_COOKIE, buffer);
  }
  /* session cookie found */
  log("Session cookie found\n");
  return MHD_YES;
}
/* initialize template map */
static template_map_queue_t *initialize_template_map(){
  return queue_fabric();
}
static void destroy_template_map(template_map_queue_t *template_map){
  if(template_map!=NULL) queue_destructor(template_map);
}
/* add template map */
static void add_template_map(template_map_queue_t *template_map, const char *keyWord, const char *replaceBy){
  if(template_map==NULL){
    log_warn("Unable to set template_map, hint: use initialize_template_map()\n");
    return;
  }
  /* create new template_map_type_t */
  template_map_type_t *temp = (template_map_type_t*)malloc(sizeof(template_map_type_t));
  memset(temp, 0, sizeof(template_map_type_t));
  temp->keyWord[0] = '\0';
  temp->replaceBy[0] = '\0';
  memcpy(temp->keyWord, keyWord, sizeof(char)*strlen(keyWord));
  memcpy(temp->replaceBy, replaceBy, sizeof(char)*strlen(replaceBy));
  /* add template_map to the queue */
  queue_insert_item_on_top(template_map, temp, sizeof(template_map_type_t), free); // the queue will free the memory on queue_destructor
}
/* load_html_template is a method that loads a html_file (path) and uses regex to replace the template values {{KEY}}=>Value */
static void load_html_template(const char *html_file, template_map_queue_t *template_map, char *buffer_output) {
  /* initialize the auxliary variables */
  char *html_contents=NULL;
  char *temp_word = NULL;
  char regex_pattern[TEMPLATE_MAP_BUFFER_SIZE+16] = "\0";
  /* read the html file contents */
  read_text_file(html_file, &html_contents);
  if(template_map == NULL){
    /* copy the html contents to the output buffer */
    strcpy(buffer_output, html_contents);
    /* free the html contents */
    free(html_contents);
    html_contents = NULL;
    return;
  }
  /* replace template arguments */
  __queue_item_t *item=NULL;
  template_map_type_t *temp_map=(template_map_type_t*)malloc(sizeof(template_map_type_t));
  /* iterate over the template_map elements */
  queue_start_generator_up(template_map);
  while((item=queue_yield(template_map)) != NULL){
    /* assing temporal map */
    memcpy(temp_map, (template_map_type_t*)item->data, sizeof(template_map_type_t));
    /* fabricate the regex */
    snprintf(regex_pattern,TEMPLATE_MAP_BUFFER_SIZE+16,"\\{\\{%s\\}\\}",temp_map->keyWord);
    /* replace the regex matches */
    // replace_regex(regex_pattern, temp_map->replaceBy, html_contents, &temp_word);
    // re_replace(html_contents, regex_pattern, &temp_word, NULL)
    
    replace_regex(html_contents, regex_pattern, temp_map->replaceBy);

    /* copy the updated value to the html contents */
    // log("html_contents: %s\n",html_contents);
    // log("temp_word: %s\n",temp_word);
    // strcpy(html_contents, temp_word);
    // /* free the temp */
    // free(temp_word);
    // temp_word = NULL;
  }
  /* finalize */
  strcpy(buffer_output,html_contents);
  free(html_contents);
  free(temp_map);
  html_contents = NULL;
  temp_map = NULL;
}

#endif