#ifndef LISTENERS_H
#define LISTENERS_H

#include <string.h>

#include <wayland-client.h>

#include "global.h"
#include "xdg-shell-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static void handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	printf("We got: %s\n", interface);
	if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		wm_base = wl_registry_bind(registry, name,
				&xdg_wm_base_interface, 1);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		layer_shell = wl_registry_bind(registry, name,
				&zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, wl_compositor_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		compositor = wl_registry_bind(registry, name,
				&wl_compositor_interface, 4);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
}

static void handle_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface,
		uint32_t serial, uint32_t width, uint32_t height)
{
	zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface, serial);
}

static void handle_close(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface)
{
	printf("We are closing, sorry budy!\n");
	window_closed = true;
}

struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = noop
};

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = handle_configure,
	.closed = handle_close
};

#endif //LISTENERS_H
