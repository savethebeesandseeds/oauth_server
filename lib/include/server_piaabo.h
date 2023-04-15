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

#define MHD_RequestTerminationCode_to_string (const char[4][40]) {\
  "TERMINATED_COMPLETED_OK","TERMINATED_WITH_ERROR","TERMINATED_TIMEOUT_REACHED","TERMINATED_DAEMON_SHUTDOWN"}
#define COOKIE_SIZE 64
#define MAX_HEADERS 64
#define MAX_FOOTERS 64
#define MAX_TEMPLATE_REPLACEMENTS  64

typedef const char* output_char_function_pointer();
typedef enum MHD_Result route_pointer(
  void *dh_cls, struct MHD_Connection *connection);
typedef struct {
  char *keyWord;
  char *replaceBy;
} HTML_TemplateKeyValue;
typedef struct {
  char *keyWord;
  char *valueWord;
} HeadersKeyValue;
typedef struct {
  char *keyWord;
  char *valueWord;
} FootersKeyValue;

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
static void log_request(
  void *cls, 
  struct MHD_Connection *connection,
  const char *url,
  const char *method, 
  const char *version,
  const char *upload_data,
  size_t *upload_data_size, void **con_cls){
  /* log path and version */
  log("--- %s[%s] request%s, for path: %s, using version %s\n", 
    ANSI_COLOR_Bright_Yellow, method, ANSI_COLOR_RESET, url, version);
  /* log request values */
  MHD_get_connection_values(connection, MHD_HEADER_KIND, &log_connection_value, (void *)&ANSI_COLOR_Red);
  MHD_get_connection_values(connection, MHD_COOKIE_KIND, &log_connection_value, (void *)&ANSI_COLOR_Magenta);
  MHD_get_connection_values(connection, MHD_POSTDATA_KIND, &log_connection_value, (void *)&ANSI_COLOR_Blue);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &log_connection_value, (void *)&ANSI_COLOR_Green);
  MHD_get_connection_values(connection, MHD_FOOTER_KIND, &log_connection_value, (void *)&ANSI_COLOR_Bright_Yellow);
}
/* session manager with cookies */
static void session_cookie(char *set_value){
  log_warn("Session Cookie needs to be set better than just random.\n");
  /* autiliary variable */
  char raw_value[COOKIE_SIZE];
  /* random method to create cookie */
  for (unsigned int i=0;i<sizeof(raw_value);i++)
    raw_value[i]='A' + (rand () % 26); /* bad PRNG! */
  raw_value[64] = '\0';
  snprintf(set_value, 2*COOKIE_SIZE, "%s=%s","SESSION", raw_value);
}
/* set session cookie */
static enum MHD_Result set_session_cookie (
  struct MHD_Connection *connection,
  struct MHD_Response *response){
  /* autiliary variable */
  char set_value[2*COOKIE_SIZE];
  /* validate the response */
  if(response==NULL)
    log_fatal("Unable to set cookie on a null response\n");
  /* reading cookies */
  const char *read_value = 
    MHD_lookup_connection_value(
      connection, MHD_COOKIE_KIND, "SESSION");
  if(read_value!=NULL){
    log("Session cookie found\n");
    return MHD_YES;
  }
  /* setting cookie */
  log("Setting Session cookie\n");
  session_cookie(set_value);
  return MHD_add_response_header(
    response, MHD_HTTP_HEADER_SET_COOKIE, set_value);
}
/* respond with HTML */
static enum MHD_Result create_html_response(
  const char* page_contents, 
  struct MHD_Connection *connection, 
  struct MHD_Response **response){
  /* verify the response is NULL */
  if(*response != NULL)
    log_fatal("Input MHD_Response on create_html_response must be a NULL pointer, just to track the pointer.\n");
  /* fill response from buffer */
  *response = MHD_create_response_from_buffer(
    strlen(page_contents), (void*)page_contents, MHD_RESPMEM_PERSISTENT);
  /* verify the response has been set */
  if(*response == NULL)
    return MHD_NO;
  /* return success */
  return MHD_YES;
}
/* respond with HTML */
static enum MHD_Result respond_with_html(
  const char* page_contents, 
  struct MHD_Connection *connection, 
  unsigned int status_code,
  HeadersKeyValue headers[MAX_HEADERS],
  FootersKeyValue footers[MAX_FOOTERS]){
  /* initialize auxiliary variables */
  const char *content_type="text/html";
  enum MHD_Result result;
  struct MHD_Response *response = NULL;
  /* initialzie an HTML response */
  result = create_html_response(
    page_contents, connection, &response);
  /* validate the result */
  if(result == MHD_NO){
    MHD_destroy_response(response);
    return MHD_NO;
  }
  /* add the response headers */
  MHD_add_response_header(
    response, "Content-Type", content_type);
  if(headers != NULL)
    for(size_t index=0; index<MAX_HEADERS && headers[index].keyWord != NULL && headers[index].valueWord != NULL; index++)
      MHD_add_response_header(response, 
        headers[index].keyWord, headers[index].valueWord);
  /* add the response footers */
  if(footers != NULL)
    for(size_t index=0; index<MAX_FOOTERS && footers[index].keyWord != NULL && footers[index].valueWord != NULL; index++)
      MHD_add_response_footer(response, 
        footers[index].keyWord, footers[index].valueWord);
  /* queue the response */
  result = MHD_queue_response(
    connection, status_code, response);
  /* log response */
  if(result == MHD_NO) 
    log_err("Unable to respond with %s\n", content_type)
  else log("--- --- %sResponding with%s: statusCode:[%d], Content-Type:[%s]\n",
    ANSI_COLOR_Bright_Yellow, ANSI_COLOR_RESET, status_code, content_type);
  /* free response */
  MHD_destroy_response(response);

  /* return result */
  return result;
}
/* create_response_from_file */
static enum MHD_Result create_response_from_file(
  int fd,
  struct MHD_Connection *connection, 
  struct MHD_Response **response){
  /* temporary variables */
  struct stat sbuf;
  /* verify the response is NULL */
  if(*response != NULL)
    log_fatal("Input MHD_Response on create_response_from_file must be a NULL pointer, just to track the pointer.\n");
  /* verify the file contetns */
  if((-1 == fd) || (0 != fstat(fd, &sbuf)) ){
    if (fd != -1) (void) close (fd);
    return respond_with_html(
      "<html><body>An internal server error has occurred!</body></html>", 
      connection, MHD_HTTP_INTERNAL_SERVER_ERROR, NULL, NULL);
  }
  /* fabric MHD_Response with binary buffer data */
  *response = MHD_create_response_from_fd_at_offset64(sbuf.st_size, fd, 0);
  /* verify the response has been set */
  if(*response==NULL)
    return MHD_NO;
  
  /* return sucess */
  return MHD_YES; 
}
/* respond with FILE */
static enum MHD_Result respond_with_file(
  int fd,
  const char *content_type,
  struct MHD_Connection *connection, 
  unsigned int status_code,
  HeadersKeyValue headers[MAX_HEADERS],
  FootersKeyValue footers[MAX_FOOTERS]){
  /* initialize auxiliary variables */
  enum MHD_Result result;
  struct MHD_Response *response = NULL;
  /* initialzie an HTML response */
  result = create_response_from_file(fd, connection, &response);
  /* validate the result */
  if(result == MHD_NO){
    MHD_destroy_response(response);
    return MHD_NO;
  }
  /* add the response headers */
  MHD_add_response_header(
    response, "Content-Type", content_type);
  if(headers != NULL)
    for(size_t index=0; index<MAX_HEADERS && headers[index].keyWord != NULL && headers[index].valueWord != NULL; index++)
      MHD_add_response_header(response, 
        headers[index].keyWord, headers[index].valueWord);
  /* add the response footers */
  if(footers != NULL)
    for(size_t index=0; index<MAX_FOOTERS && footers[index].keyWord != NULL && footers[index].valueWord != NULL; index++)
      MHD_add_response_footer(response, 
        footers[index].keyWord, footers[index].valueWord);
  /* queue the response */
  result = MHD_queue_response(
    connection, status_code, response);
  if(result == MHD_NO) 
    log_err("Unable to respond with %s\n", content_type)
  else log("--- --- %sResponding with%s: statusCode:[%d], Content-Type:[%s]\n",
      ANSI_COLOR_Bright_Yellow, ANSI_COLOR_RESET, status_code, content_type)
  /* free response */
  MHD_destroy_response(response);
  /* return result */
  return result;
}
/* load_html_template is a method that loads a html_file (path) and uses regex to replace the template values {{KEY}}=>Value */
static void load_html_template(const char *html_file, HTML_TemplateKeyValue *templateKeyValues, char *output) {
  /* initialize the auxliary variables */
  char *html_contents=NULL;
  char *temp=NULL;
  char regex_pattern[2048] = "\0";
  /* read the html file contents */
  read_text_file(html_file, &html_contents);
  /* loop over all templateKeyValues */
  for (size_t index = 0; index < MAX_TEMPLATE_REPLACEMENTS && templateKeyValues[index].keyWord != NULL && templateKeyValues[index].replaceBy != NULL; index++) {
    /* fabricate the regex */
    sprintf(regex_pattern,"\\{\\{%s\\}\\}",templateKeyValues[index].keyWord);
    /* replace the regex matches */
    replace_regex(regex_pattern, templateKeyValues[index].replaceBy, html_contents, &temp);
    /* copy the updated value to the html contents */
    strcpy(html_contents, temp);
    /* free the temp */
    free(temp);
    temp = NULL;
  }
  /* finalize */
  strcpy(output,html_contents);
  free(html_contents); 
}

#endif