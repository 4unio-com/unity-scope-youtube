#include <glib.h>
#include <libaccounts-glib/accounts-glib.h>
//#include <libsignon-glib/signon-glib.h>
#include<cstdio>
#include<cstring>

typedef struct _AuthContext AuthContext;
struct _AuthContext {
    AgManager *manager;
    char *service_name;
    AgAccountService *account_service;
    AgAuthData *auth_data;
//    SignonAuthSession *session;

    GVariant *auth_params;
    GVariant *session_data;
};

static void lookup_account_service(AuthContext *ctx) {
    GList *account_services = ag_manager_get_enabled_account_services(ctx->manager);
    GList *tmp;
    printf("Y\n");
    for (tmp = account_services; tmp != NULL; tmp = tmp->next) {
        AgAccountService *acct_svc = AG_ACCOUNT_SERVICE(tmp->data);
        AgService *service = ag_account_service_get_service(acct_svc);
        printf("%s\n", ag_service_get_name(service));
        /*
        if (!strcmp(ctx->service_name, ag_service_get_name(service))) {
            ctx->account_service = g_object_ref(acct_svc);
            break;
        }
        */
    }
    g_list_foreach(account_services, (GFunc)g_object_unref, NULL);
    g_list_free(account_services);
/*
    if (ctx->account_service != NULL) {
        login_service(ctx);
    }
    */
}

static void account_enabled_cb(AgManager *manager, guint account_id, void *user_data) {
    AuthContext *ctx = (AuthContext *)user_data;

    printf("enabled_cb account_id=%u\n", account_id);

    if (ctx->account_service != NULL &&
        !ag_account_service_get_enabled(ctx->account_service)) {
        //logout_service(ctx);
    }
    lookup_account_service(ctx);
}

static gboolean
setup_context(void *user_data) {
    AuthContext *ctx = (AuthContext *)user_data;
    printf("X\n");
    lookup_account_service(ctx);
    g_signal_connect(ctx->manager, "enabled-event",
                     G_CALLBACK(account_enabled_cb), ctx);
    return FALSE;
}

int main(int argc, char **argv) {
    GMainLoop *ml = g_main_loop_new(g_main_context_default(), FALSE);
    AuthContext *ctx = g_new0(AuthContext, 1);
    ctx->manager = ag_manager_new();
    ctx->service_name = g_strdup("youtube");

    g_idle_add(setup_context, ctx);
    g_main_loop_run(ml);
    return 0;
}
