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
  /* initialize routing_data_t struct */
  routing_data_t route_data = {};
  route_data.dh_cls=dh_cls;
  route_data.connection=connection;
  route_data.url=url;
  route_data.method=method;
  route_data.version=version;
  route_data.upload_data=upload_data;
  route_data.upload_data_size=upload_data_size;
  route_data.con_cls=con_cls;
  /* log requests */
  log_request(&route_data);
  /* respond to request allowing access to the public methods */
  return route_coordinator(&route_data)(&route_data);
}
#endif