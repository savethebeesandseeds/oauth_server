#ifndef __SERVER_TLS
#define __SERVER_TLS
#include <microhttpd.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "util_piaabo.h"
#include "server_piaabo.h"
#include "server_endpoints.h"

/* extract the clients tls session */
static gnutls_session_t get_tls_session(
  struct MHD_Connection *connection){
  const union MHD_ConnectionInfo *ci= MHD_get_connection_info(
    connection,MHD_CONNECTION_INFO_GNUTLS_SESSION);
  return (gnutls_session_t)ci->tls_session;
}
/* Get the client's certificate */
static gnutls_x509_crt_t get_client_certificate(
  gnutls_session_t tls_session){
  /* temporal variables */
  unsigned int listsize;
  const gnutls_datum_t * pcert;
  gnutls_certificate_status_t client_cert_status;
  gnutls_x509_crt_t client_cert;
  /* validation */
  if(tls_session == NULL)
    return NULL;
  /* verify the certificate */
  if(gnutls_certificate_verify_peers2(tls_session,(unsigned int*)&client_cert_status))
    return NULL; // Invalid or absent certificate
  pcert = gnutls_certificate_get_peers(tls_session,&listsize);
  if((pcert == NULL) ||(listsize == 0)){
    log_err("Failed to retrieve client certificate chain\n");
    return NULL;
  }
  if(gnutls_x509_crt_init(&client_cert)){
    log_err("Failed to initialize client certificate\n");
    return NULL;
  }
  /* Note that by passing values between 0 and listsize here, you can get access to the CA's certs */
  if(gnutls_x509_crt_import(client_cert,&pcert[0],GNUTLS_X509_FMT_DER)){
    log_err("Failed to import client certificate\n");
    gnutls_x509_crt_deinit(client_cert);
    return NULL;
  }
  return client_cert;
}
/* Get the distinguished name from the client's certificate */
char *cert_auth_get_dn(
  gnutls_x509_crt_t client_cert){
  size_t lbuf=0;
  gnutls_x509_crt_get_dn(client_cert, NULL, &lbuf);
  char* buf = (char*) malloc(lbuf);
  if(buf == NULL){
    log_err("Failed to allocate memory for certificate dn\n");
    return NULL;
  }
  gnutls_x509_crt_get_dn(client_cert, buf, &lbuf);
  return buf;
}
/**
 * Get the alternative name of specified type from the client's certificate
 *
 * @param client_cert the client certificate
 * @param nametype The requested name type
 * @param index The position of the alternative name if multiple names are
 * 			matching the requested type, 0 for the first matching name
 * @return NULL if no matching alternative name could be found, a pointer
 * 			to the alternative name if found
 */
char *MHD_cert_auth_get_alt_name(
  gnutls_x509_crt_t client_cert,
  int nametype,
  unsigned int index){
  /* temporal variables */
  char* buf;
  size_t lbuf;
  unsigned int seq;
  unsigned int subseq;
  unsigned int type;
  int result;
  /* loop */
  subseq = 0;
  for(seq=0;;seq++){
    lbuf = 0;
    result = gnutls_x509_crt_get_subject_alt_name2(
      client_cert, seq, NULL, &lbuf, &type, NULL);
    if(result == GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE)
      return NULL;
    if(nametype != (int) type)
      continue;
    if(subseq == index)
      break;
    subseq++;
  }
  /* allocate buffer */
  buf = (char*)malloc(lbuf);
  if(buf == NULL){
    log_err("Failed to allocate memory for certificate alt name\n");
    return NULL;
  }
  /* extract buffer */
  result = gnutls_x509_crt_get_subject_alt_name2(
    client_cert,seq,buf,&lbuf,NULL, NULL);
  /* validate result */
  if(result != nametype){
    log_err("Unexpected return value from gnutls: %d\n",result);
    free(buf);
    return NULL;
  }
  /* return sucess */
  return buf;
}
// gnutls_x509_crt_deinit(client_cert);
/* method to process request with server having tls auth and client having tls auth */
static enum MHD_Result answer_to_connection_tls_auth(
  void *dh_cls, 
  struct MHD_Connection *connection,
  const char *url,
  const char *method, 
  const char *version,
  const char *upload_data,
  size_t *upload_data_size, 
  void **con_cls){
  /* initial only method GET */
  if(0 != strcmp(method, "GET"))
    return MHD_NO;
  /* initial communication */
  if(NULL == *con_cls){
    *con_cls = connection;
    return MHD_YES;
  }
  /* initialize routing_data_t struct */
  routing_data_t route_data = initialize_routing_data(dh_cls, connection, url,method, version);
  /* aditional fields */
  route_data.upload_data=upload_data;
  route_data.upload_data_size=upload_data_size;
  route_data.con_cls=con_cls;
  /* check if the client send a ssl/tls cert */
  route_data.tls_session = get_tls_session(connection);
  if(route_data.tls_session==NULL){
    log_warn("The client is not trying to reach this server, has no tls_session\n");
    return ask_authentication(&route_data);
  }
  /* retrive the ssl/tls cert */
  route_data.client_cert = get_client_certificate(route_data.tls_session);
  if(route_data.client_cert==NULL){
    log_warn("No tls certficate found to authenticate client request\n");
    return ask_authentication(&route_data);
  }
  /* retrive the client distinguished name */
  route_data.distinguished_name = cert_auth_get_dn(route_data.client_cert);
  /* retrive the client alternative name */
  route_data.alt_name = MHD_cert_auth_get_alt_name(route_data.client_cert,0,0);
  /* log requests */
  log_request(&route_data);
  /* respond to request allowing access to the secure methods */
  return secure_route_coordinator(&route_data)(&route_data);
}
#endif