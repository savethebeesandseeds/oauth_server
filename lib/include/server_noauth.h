#ifndef __SERVER_NOAUTH
#define __SERVER_NOAUTH
#include <microhttpd.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "util_piaabo.h"
#include "server_piaabo.h"
#include "server_endpoints.h"

/* method to process request with server noauth */
static enum MHD_Result answer_to_connection_noauth(
  void *dh_cls, 
  struct MHD_Connection *connection,
  const char *url,
  const char *method, 
  const char *version,
  const char *upload_data,
  size_t *upload_data_size, 
  void **con_cls){
  /* initial communication */
  if(NULL == *con_cls){
    *con_cls = connection;
    return MHD_YES;
  }
  /* log requests */
  log_request(dh_cls, connection, url, method, version, upload_data, upload_data_size, con_cls);
  /* respond to request allowing access to the secure methods */
  return route_coordinator(url, method, version)(dh_cls, connection);
}
#endif