#ifndef __RESPONSE_PIAABO
#define __RESPONSE_PIAABO

#include <fcntl.h>
#include <sys/stat.h>
#include <microhttpd.h>

#include "util_piaabo.h"
#include "file_piaabo.h"
#include "queue_piaabo.h"

#define HEADER_BUFFER_SIZE (1<<16) /* 65536: this is the maximun length a header can have */
#define FOOTER_BUFFER_SIZE (1<<16) /* 65536: this is the maximun length a footer can have */

typedef __queue_t response_headers_queue_t;
typedef struct {char key[HEADER_BUFFER_SIZE]; char value[HEADER_BUFFER_SIZE];} response_header_type_t;
typedef __queue_t response_footers_queue_t;
typedef struct {char key[FOOTER_BUFFER_SIZE]; char value[FOOTER_BUFFER_SIZE];} response_footer_type_t;

typedef struct {
  enum MHD_Result mhd_result;
  struct MHD_Response *mhd_response;
  struct MHD_Connection *mhd_connection;
  response_footers_queue_t *footers;
  response_footers_queue_t *headers;
  unsigned int status_code;
} server_response_t;

/* initialize server response */
static server_response_t *initialize_server_response(struct MHD_Connection *mhd_connection){
  /* allocate space for the response */
  server_response_t *response = (server_response_t *)malloc(sizeof(server_response_t));
  /* initialize the response */
  response->mhd_result = MHD_YES;
  response->mhd_response = NULL;
  response->footers = queue_fabric();
  response->headers = queue_fabric();
  response->status_code = MHD_HTTP_OK;
  response->mhd_connection = mhd_connection;
  /* return the response */
  return response;
}
/* destroy server response */
static void destroy_server_response(server_response_t *response){
  /* destroy the response */
  if(response->mhd_response!=NULL) MHD_destroy_response(response->mhd_response);
  if(response->footers!=NULL) queue_destructor(response->footers);
  if(response->headers!=NULL) queue_destructor(response->headers);
  /* free the response */
  free(response);
}
/* set the response headers */
static void set_response_headers(server_response_t *response){
  __queue_item_t *item=NULL;
  response_header_type_t *temp=(response_header_type_t*)malloc(sizeof(response_header_type_t));
  queue_start_generator_up(response->headers);
  while((item=queue_yield(response->headers)) != NULL){
    memcpy(temp, (response_header_type_t*)item->data, sizeof(response_header_type_t));
    if(temp!=NULL)
      if(MHD_NO == MHD_add_response_header(response->mhd_response, temp->key, temp->value))
        response->mhd_result = MHD_NO;
  }
  free(temp);
}
/* set the response footers */
static void set_response_footers(server_response_t *response){
  __queue_item_t *item=NULL;
  response_footer_type_t *temp=(response_footer_type_t*)malloc(sizeof(response_footer_type_t));
  queue_start_generator_up(response->footers);
  while((item=queue_yield(response->footers)) != NULL){
    memcpy(temp, (response_footer_type_t*)item->data, sizeof(response_footer_type_t));
    if(temp!=NULL)
      if(MHD_NO == MHD_add_response_header(response->mhd_response, temp->key, temp->value))
        response->mhd_result = MHD_NO;
  }
  free(temp);
}
/* add response header functionality */
static void add_response_header(server_response_t *response, const char *dkey, const char *dvalue){
  if(response->headers==NULL){
    log_warn("Unable to set response header, hint: use initialize_headers()\n");
    return;
  }
  /* create new response_header_type_t */
  response_header_type_t *dheader = (response_header_type_t*) malloc(sizeof(response_header_type_t));
  memset(dheader, 0, sizeof(response_header_type_t));
  dheader->key[0] = '\0';
  dheader->value[0] = '\0';
  memcpy(dheader->key, dkey, sizeof(char)*strlen(dkey));
  memcpy(dheader->value, dvalue, sizeof(char)*strlen(dvalue));
  /* add dheader to the queue */
  queue_insert_item_on_top(response->headers, dheader, sizeof(response_header_type_t), free); // the queue will free the memory on queue_destructor
}
/* add response footer functionality */
static void add_response_footer(server_response_t *response, const char *dkey, const char *dvalue){
  if(response->footers==NULL){
    log_warn("Unable to set response footer, hint: use initialize_footers()\n");
    return;
  }
  /* create new response_footer_type_t */
  response_footer_type_t *dfooter = (response_footer_type_t*) malloc(sizeof(response_footer_type_t));
  memset(dfooter, 0, sizeof(response_footer_type_t));
  dfooter->key[0] = '\0';
  dfooter->value[0] = '\0';
  memcpy(dfooter->key, dkey, sizeof(char)*strlen(dkey));
  memcpy(dfooter->value, dvalue, sizeof(char)*strlen(dvalue));
  /* add dfooter to the queue */
  queue_insert_item_on_top(response->footers, dfooter, sizeof(response_footer_type_t), free); // the queue will free the memory on queue_destructor
}
/* set status code */
static void set_status_code(server_response_t *response, int status_code){
  if(response==NULL){
    log_err("Unable to set status code, hint: use initialize_server_response()\n");
    return;
  }
  response->status_code = status_code;
}
/* perormance response */
static enum MHD_Result perform_response(server_response_t *response, const char *content_type){
  /* validate the response */
  if(response->mhd_result == MHD_NO){
    destroy_server_response(response);
    return MHD_NO;
  }
  /* temporal variables */
  enum MHD_Result temp_result;
  /* content type response header */
  add_response_header(response, "Content-Type", content_type);
  /* set the response headers */
  set_response_headers(response);
  /* add the response footers */
  set_response_footers(response);
  /* queue the response */
  temp_result = MHD_queue_response(
      response->mhd_connection, 
      response->status_code, 
      response->mhd_response);
  /* queue the response */
  if(temp_result == MHD_NO){
    /* respoond with error */
    log_err("Unable to respond with %s\n", content_type)
    response->mhd_result = MHD_NO;
  } else 
    log("--- --- %sResponding with%s: statusCode:[%d], Content-Type:[%s]\n",
      ANSI_COLOR_Bright_Yellow, ANSI_COLOR_RESET, response->status_code, content_type)

  /* finalize server response */
  destroy_server_response(response);

  /* return result */
  return temp_result;
}
/* create HTML response */
static server_response_t *create_html_response(struct MHD_Connection *connection, const char* page_contents){
  /* initialize the server response */
  server_response_t* response = initialize_server_response(connection);
  /* fill response from buffer */
  response->mhd_response = MHD_create_response_from_buffer(
    strlen(page_contents), (void*)page_contents, MHD_RESPMEM_MUST_COPY);
  /* verify the response has been set */
  if(response->mhd_response == NULL){
    /* return error */
    log_err("Unable to create html response. hint: page_contents strlen(%ld).\n", strlen(page_contents));
    response->mhd_result = MHD_NO;
  }
  return response;
}
/* respond with HTML */
static enum MHD_Result respond_with_html(server_response_t *response){
  return perform_response(response, "text/html");
}
/* quick HTML response */
static enum MHD_Result quick_html_response(struct MHD_Connection *connection, const char* page_contents){
  /* intialize html response */
  server_response_t *response=create_html_response(connection, page_contents);
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* respond with file */
  return respond_with_html(response);
}
/* throw internal server error */
static enum MHD_Result respond_internal_server_error(struct MHD_Connection *connection){
  /* create response */
  const char* page_contents = "<html><body>An internal server error has occurred!</body></html>";
  /* create response */
  server_response_t *response = create_html_response(connection, page_contents);
  /* set status code */
  set_status_code(response, MHD_HTTP_INTERNAL_SERVER_ERROR);
  /* respond with HTML */
  return respond_with_html(response);
}
/* create FILE response */
static server_response_t *create_response_from_file(struct MHD_Connection *connection, int fd){
  /* temporary variables */
  struct stat sbuf;
  /* initialize the server response */
  server_response_t* response = initialize_server_response(connection);
  /* verify the file contetns */
  if((-1 == fd) || (0 != fstat(fd, &sbuf)) ){
    if (fd != -1) (void) close (fd);
    log_err("Unable to read file contents while creating file response.\n");
    response->mhd_result = MHD_NO;
    return response;
  }
  /* fabric MHD_Response with binary buffer data */
  response->mhd_response = MHD_create_response_from_fd_at_offset64(sbuf.st_size, fd, 0);
  /* verify the response has been set */
  if(response->mhd_response == NULL){
    response->mhd_result = MHD_NO;
    log_err("Unable to create response from file.\n");
  }
  /* return sucess */
  return response; 
}
/* respond with FILE */
static enum MHD_Result respond_with_file(server_response_t *response, const char* content_type){
  return perform_response(response, content_type);
}
/* quick FILE response */
static enum MHD_Result quick_file_response(struct MHD_Connection *connection, int fd, const char* content_type){
  /* initialize response from file */
  server_response_t *response=create_response_from_file(connection, fd);
  /* set status code */
  set_status_code(response, MHD_HTTP_OK);
  /* respond with file */
  return respond_with_file(response, content_type);
}
#endif