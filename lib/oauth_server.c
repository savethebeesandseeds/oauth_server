// Author Waajacu
// License: MIT
// Version: 1.0.0
// Created: 2023-04-08 09:00:00
// Description: Hello World in C
// Language: C
// Path: tauhba_server.c
// Compiler: gcc
// Build: make tauhba_server

#include "stdio.h"
#include "server_noauth.h"
#include "server_piaabo.h"
#include "util_piaabo.h"
#include "file_piaabo.h"

#define SERVERPORT 8888
#define SERVERCERTKEYFILE "access/server.key.pem"
#define SERVERCERTPEMFILE "access/server.pem" // used by the server to authenticate it self
#define CLIENTCERTPEMFILE "access/client.pem" // used by the server to verify authorized clients

/* start a server that 
  - indetifies it self with a tls file and, 
  - acceps request only from clientes authenticated with tls files */
struct MHD_Daemon *start_noauth_server(uint16_t port){
  /* return server daemon */
  return MHD_start_daemon(
    MHD_USE_DEBUG | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION, port, 
    on_client_connect, NULL,
    &answer_to_connection_noauth, NULL, 
    MHD_OPTION_CONNECTION_TIMEOUT,(unsigned int) 15,
    MHD_OPTION_NOTIFY_COMPLETED, &on_request_completed, NULL,
    MHD_OPTION_END);
}
/* run secured tls server */
int run_server(){
  /* start the tls server */
  struct MHD_Daemon *daemon = start_noauth_server(SERVERPORT);
  /* validate server daemon */
  if(NULL == daemon){
    /* log unable to start server error */
    log_err("Unable to start server %s MHD.\n",MHD_HTTP_VERSION_1_1);
    /* close on error */
    return EXIT_FAILURE;
  }
  /* loop */
  log("[Running server on port %s%d%s]...\n",ANSI_COLOR_Red,SERVERPORT,ANSI_COLOR_RESET);
  getchar();
  log("[Closing server]...\n");
  /* stop server daemon */
  MHD_stop_daemon(daemon);
  /* return success */
  return EXIT_SUCCESS; 
}
/* main */
int main(int argc, char *argv[]){
  return run_server();
}