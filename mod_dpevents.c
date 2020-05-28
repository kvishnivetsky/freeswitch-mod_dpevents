/*.
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *.
 * Anthony Minessale II <anthm@freeswitch.org>
 * Neal Horman <neal at wanlink dot com>
 * <Enter your name and e-mail here>
 *
 * mod_dpevents.c -- <Enter your module short description here>
 *
 */
#include <switch.h>

static struct {
	switch_mutex_t *listener_mutex;
	switch_event_node_t *node;
	int debug;
} globals;


struct listener {
	char *event_name;
	char *event_subclass;
	char *app;
	char *app_arg;
	switch_core_session_t *session;
	struct listener *next;
	int is_api;
};

typedef struct listener listener_t;


static struct {
	switch_mutex_t *list_mutex;
	listener_t *listeners;
} listen_list;

static void add_listener(listener_t *listener)
{
	/* add me to the listeners so I get events */
	switch_mutex_lock(globals.listener_mutex);
	listener->next = listen_list.listeners;
	listen_list.listeners = listener;
	switch_mutex_unlock(globals.listener_mutex);
}

static void remove_listener(listener_t *listener)
{
	listener_t *l, *last = NULL;

	switch_mutex_lock(globals.listener_mutex);
	if (globals.debug) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Remove listener for event %s\n", listener->event_name);
	}
	for (l = listen_list.listeners; l; l = l->next) {
		if (l == listener) {
			if (last) {
				last->next = l->next;
			} else {
				listen_list.listeners = l->next;
			}
		}
		last = l;
	}
	switch_mutex_unlock(globals.listener_mutex);
}

#define ARGS_EXPAND switch_event_expand_headers(event, switch_string_replace(switch_channel_expand_variables(switch_core_session_get_channel(l->session), switch_string_replace(l->app_arg, "^{", "${")), "!{", "${"))

static void event_handler(switch_event_t *event)
{
	listener_t *l, *lp = NULL;
	char *event_name, *event_subclass, *event_uuid = NULL;

	// Check if event is a valid pointer
	switch_assert(event != NULL);

	event_name = switch_event_get_header(event, "Event-Name");
	event_subclass = switch_event_get_header(event, "Event-Subclass");
	event_uuid = switch_event_get_header(event, "Unique-Id");

	if (globals.debug) {
	    if (event_subclass)
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "New event %s/%s\n", event_name, event_subclass);
	    else
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "New event %s\n", event_name);
	}

	if (event_uuid) {
		switch_mutex_lock(globals.listener_mutex);

		lp = listen_list.listeners;
		while (lp) {

			l = lp;
			lp = lp->next;

			if (l == NULL) {
				switch_mutex_unlock(globals.listener_mutex);
				break;
			}

			if ((!strcmp(event_uuid, switch_core_session_get_uuid(l->session)))&&(!strcmp(event_name, "CHANNEL_DESTROY"))) {
			    remove_listener(l);
			    continue;
			}

			if (!strcmp(event_name, "CUSTOM")) {
				if ( (!strcmp(event_uuid, switch_core_session_get_uuid(l->session)))&&(!strcmp(event_name, l->event_name))&&(!strcmp(event_subclass, l->event_subclass)) ) {
					if (globals.debug) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Execute %s(%s)\n", l->app, ARGS_EXPAND);
					}
					if (l->is_api) {
						switch_stream_handle_t stream = { 0 };
						SWITCH_STANDARD_STREAM(stream);
						if (l->app_arg)
							switch_api_execute(l->app, ARGS_EXPAND, NULL, &stream);
						else
							switch_api_execute(l->app, NULL, NULL, &stream);
						if (globals.debug) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "API returned: %s\n", (char *)stream.data);
						}
						free(stream.data);
					} else {
						if (l->app_arg)
						    switch_core_session_execute_application(l->session, l->app, ARGS_EXPAND);
						else
						    switch_core_session_execute_application(l->session, l->app, NULL);
					}
				}
			} else {
				if ( (!strcmp(event_uuid, switch_core_session_get_uuid(l->session)))&&(!strcmp(event_name, l->event_name))) {
					if (globals.debug) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Execute %s(%s)\n", l->app, ARGS_EXPAND);
					}
					if (l->is_api) {
						switch_stream_handle_t stream = { 0 };
						SWITCH_STANDARD_STREAM(stream);
						if (l->app_arg)
						    switch_api_execute(l->app, ARGS_EXPAND, NULL, &stream);
						else
						    switch_api_execute(l->app, NULL, NULL, &stream);
						if (globals.debug) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "API returned: %s\n", (char *)stream.data);
						}
						free(stream.data);
					} else {
						if (l->app_arg)
							switch_core_session_execute_application(l->session, l->app, ARGS_EXPAND);
						else
							switch_core_session_execute_application(l->session, l->app, NULL);
					}
				}
			}
		}
		if (globals.debug) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "End of handler list\n");
		}

		switch_mutex_unlock(globals.listener_mutex);
	}
}

SWITCH_STANDARD_APP(bind_event_app_function)
{
	char *mydata = NULL;
	unsigned int argc = 0;
	char *argv[80] = { 0 };
	listener_t *listener = NULL;
//	switch_channel_t *channel = switch_core_session_get_channel(session);

	if (data && (mydata = switch_core_session_strdup(session, data))) {
		argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0])));
	}

	if (argc < 2) {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "bind_event application needs at lat 2 argments!!!\n");
	    return;
	}
	if (strcmp(argv[0], "CUSTOM")) {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Binding event %s to execute application %s\n", argv[0], argv[1]);
	} else {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Binding event %s::%s to execute application %s\n", argv[0], argv[1], argv[2]);
	}

	if (!(listener = switch_core_session_alloc(session, sizeof(*listener)))) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Memory Allocation Error\n");
		return;
	}

	listener->session = session;
	listener->event_name = switch_core_session_strdup(session, argv[0]);
	if (!strcmp(argv[0], "CUSTOM")) {
		listener->event_subclass = switch_core_session_strdup(session, argv[1]);
		listener->app = switch_core_session_strdup(session, argv[2]);
		if (argc > 3) {
			for(int i=4;i<argc;i++) {
			    *(argv[i]-1) = ' ';
			}
			listener->app_arg = switch_core_session_strdup(session, argv[3]);
		}
	} else {
		listener->app = switch_core_session_strdup(session, argv[1]);
		if (argc > 2) {
			for(int i=3;i<argc;i++) {
			    *(argv[i]-1) = ' ';
			}
			listener->app_arg = switch_core_session_strdup(session, argv[2]);
		}
	}
	add_listener(listener);
}

SWITCH_STANDARD_APP(bind_event_api_function)
{
	char *mydata = NULL;
	unsigned int argc = 0;
	char *argv[80] = { 0 };
	listener_t *listener = NULL;
//	switch_channel_t *channel = switch_core_session_get_channel(session);

	if (data && (mydata = switch_core_session_strdup(session, data))) {
		argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0])));
	}

	if (argc < 2) {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "bind_event application needs at lat 2 argments!!!\n");
	    return;
	}
	if (strcmp(argv[0], "CUSTOM")) {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Binding event %s to execute application %s\n", argv[0], argv[1]);
	} else {
	    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Binding event %s::%s to execute application %s\n", argv[0], argv[1], argv[2]);
	}

	if (!(listener = switch_core_session_alloc(session, sizeof(*listener)))) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Memory Allocation Error\n");
		return;
	}

	listener->session = session;
	listener->event_name = switch_core_session_strdup(session, argv[0]);
	if (!strcmp(argv[0], "CUSTOM")) {
		listener->event_subclass = switch_core_session_strdup(session, argv[1]);
		listener->app = switch_core_session_strdup(session, argv[2]);
		if (argc > 3) {
			for(int i=4;i<argc;i++) {
			    *(argv[i]-1) = ' ';
			}
			listener->app_arg = switch_core_session_strdup(session, argv[3]);
		}
	} else {
		listener->app = switch_core_session_strdup(session, argv[1]);
		if (argc > 2) {
			for(int i=3;i<argc;i++) {
			    *(argv[i]-1) = ' ';
			}
			listener->app_arg = switch_core_session_strdup(session, argv[2]);
		}
	}
	listener->is_api = 1;
	add_listener(listener);
}

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_dpevents_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_dpevents_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_dpevents_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime).
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_dpevents, mod_dpevents_load, mod_dpevents_shutdown, NULL);

/* Macro expands to: switch_status_t mod_dpevents_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_dpevents_load)
{
	switch_application_interface_t *app_interface;

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Module mod_dpevents loading...\n");

	// Clean globals
	memset(&globals, 0, sizeof(globals));
	// Init global mutex
	switch_mutex_init(&globals.listener_mutex, SWITCH_MUTEX_NESTED, pool);

	// Clean listener list
	memset(&listen_list, 0, sizeof(listen_list));
	// Init listeners list mutex
	switch_mutex_init(&listen_list.list_mutex, SWITCH_MUTEX_NESTED, pool);

	if (switch_event_bind_removable(modname, SWITCH_EVENT_ALL, SWITCH_EVENT_SUBCLASS_ANY, event_handler, NULL, &globals.node) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event listener!\n");
		return SWITCH_STATUS_GENERR;
	}

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_APP(app_interface,
		    "bind_event_app",
		    "Bind event to execute dialplan application",
		    "bind_event_app",
		    bind_event_app_function,
		    "",
		    SAF_SUPPORT_NOMEDIA | SAF_ZOMBIE_EXEC | SAF_ROUTING_EXEC);
	SWITCH_ADD_APP(app_interface,
		    "bind_event_api",
		    "Bind event to execute API function",
		    "bind_event_api",
		    bind_event_api_function,
		    "",
		    SAF_SUPPORT_NOMEDIA | SAF_ZOMBIE_EXEC | SAF_ROUTING_EXEC);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_dpevents_shutdown()
*/
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_dpevents_shutdown)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Module mod_dpevents unloading...\n");
	/* Cleanup dynamically allocated config settings */

	switch_event_unbind(&globals.node);

	return SWITCH_STATUS_SUCCESS;
}


/*
  If it exists, this is called in it's own thread when the module-load completes
  If it returns anything but SWITCH_STATUS_TERM it will be called again automatically
  Macro expands to: switch_status_t mod_dpevents_runtime()
SWITCH_MODULE_RUNTIME_FUNCTION(mod_dpevents_runtime)
{
	while(looping)
	{
	    switch_cond_next();
	}
	return SWITCH_STATUS_TERM;
}
*/

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
