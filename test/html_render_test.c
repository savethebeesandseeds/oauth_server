#include "server_piaabo.h"

#define HOME_HTML "resources/home.html"
#define LOGGING_HTML "resources/logging.html"
#define OAUTH_LANDING_HTML "resources/oauth_redirect.html"

/*  html tender test */
int main(int argc, char const *argv[]){
  /* allocate render variables */
  char buffer[1<<16]="\0";
  /* initialize a template_map */
  template_map_queue_t *template_map=initialize_template_map();
  /* list the template arguments */
  add_template_map(template_map, "OAUTH_URL", "waka");
  /* load and replace html template arguments */
  load_html_template(LOGGING_HTML, template_map, buffer);
  /* free the template_map */
  destroy_template_map(template_map);

  return 0;
}