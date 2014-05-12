#include <glib.h>
#include <libaccounts-glib/accounts-glib.h>
//#include <libsignon-glib/signon-glib.h>
#include<cstdio>

int main(int argc, char **argv) {
  auto m = ag_manager_new();
  GList *list = ag_manager_get_enabled_account_services(m);
  while(list != nullptr) {
    AgAccountService *as = AG_ACCOUNT_SERVICE(list->data);
    AgService *s = ag_account_service_get_service(as);
    printf("%s %s\n", ag_service_get_name(s), ag_service_get_display_name(s));
    list = list->next;
  }
  return 0;
}
