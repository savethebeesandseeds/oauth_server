#include "requests_piaabo.hpp"
#include <map>

int main(int argc, char const *argv[])
{
  /* Request parameters */
  const char *METHOD = "GET";
  const char *URL = "https://www.oauth.com/playground/auth-dialog.html";
  std::map<std::string, std::string> get_params = {
    {"response_type", "code"},
    {"client_id", "7CCUISrBbfkLdJ46p7RohFKo"},
    {"redirect_uri", "http://localhost:8888/"},
    {"scope", "photo+offline_access"},
    {"state", "du9JpZqN6LKO_gX4"}
  };
  std::vector<std::string> headers={};
  REQUESTS::memory request_text_response = {
    .response=NULL,
    .size=0
  };
  /* Request configuration */
  REQUESTS::REQUEST_CONFIG get_request_config = REQUESTS::REQUEST_CONFIG(
    METHOD, /* method */
    URL,  /* url */
    data_callback_text, /* data_callback */
    (void*)&request_text_response, /* arg_data_callback */
    header_callback, /* headers_callback */
    NULL, /* arg_headers_callback */
    &headers, /* headers */
    &get_params, /* get params */
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
  REQUESTS::request(get_request_config);
  fprintf(stdout,"Response: \n%s\n",request_text_response.response);
  free(request_text_response.response);

  return 0;
}