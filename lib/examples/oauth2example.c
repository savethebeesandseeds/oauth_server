#include "stdio.h"
#include "requests_piaabo.h"
#include "queue_piaabo.h"

int main(int argc, char const *argv[])
{
  /* Request parameters */
  const char *METHOD = "GET";
  const char *URL = "https://www.oauth.com/playground/auth-dialog.html";
  
  headers_queue_t *headers = queue_fabric();
  params_queue_t *params = queue_fabric();

  queue_insert_item_on_top(params, &(param_type_t){"response_type", "code"}, NULL);
  queue_insert_item_on_top(params, &(param_type_t){"client_id", "7CCUISrBbfkLdJ46p7RohFKo"}, NULL);
  queue_insert_item_on_top(params, &(param_type_t){"redirect_uri", "http://localhost:8888/"}, NULL);
  queue_insert_item_on_top(params, &(param_type_t){"scope", "photo+offline_access"}, NULL);
  queue_insert_item_on_top(params, &(param_type_t){"state", "du9JpZqN6LKO_gX4"}, NULL);
  
  request_memory_t request_text_response = {
    .response=NULL,
    .size=0
  };

  /* Request configuration */
  request_config_t *request_config = build_request_config(
    METHOD, /* method */
    URL,  /* url */
    data_callback_text, /* data_callback */
    (void*)&request_text_response, /* arg_data_callback */
    header_callback, /* headers_callback */
    NULL, /* arg_headers_callback */
    headers, /* headers */
    params, /* get params */
    NULL, /* post_fileds */
    (size_t)0, /* post_fileds_size */
    true, /* curl_verbose */
    true, /* trust_self_signed_server_ssl_tls */
    NULL, /* ssl_cert_type */
    NULL, /* ssl_cert */
    NULL, /* ssl_key */
    NULL  /* ssl_key_password */
  );
  /* Make the request */
  http_request(request_config);
  fprintf(stdout,"Response: \n%s\n",request_text_response.response);
  free(request_text_response.response);

  return 0;
}