#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <wayland-client.h>

#include "shm.h"
#include "xdg-shell-client-protocol.h"

static const unsigned char image[] = {
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF
};
static const int width = 2;
static const int height = 2;

static bool running = true;
static void *shm_data = NULL;

static struct wl_display *display = NULL;
static struct wl_registry *registry = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_surface *surface = NULL;
static struct wl_shm *shm = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;

static void noop() {/* Empty */}

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version)
{
	printf("we got: %s\n", interface);
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		printf("\tbinding compositor\n");
		compositor = wl_registry_bind(registry, name,
				&wl_compositor_interface, 1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		printf("\tbinding wm_base\n");
		xdg_wm_base = wl_registry_bind(registry, name,
				&xdg_wm_base_interface, 1);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		printf("\tbinding wl_shm\n");
		shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}

}

static void on_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	printf("\tWe are closing, sorry!\n");
	running = false;
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name)
{
	printf("Someone is being removed\n");
}

static struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove
};

static struct xdg_toplevel_listener toplevel_listener = {
	.configure = noop,
	.close = on_close
};

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static struct wl_buffer *create_buffer()
{
	size_t size = sizeof(image);

	if (shm == NULL) {
		perror("nope 1!");
		return NULL;
	}

	// create shared memory
	int fd = create_shm_file(size);
	if (fd < 0) {
		perror("nope 1!");
		return NULL;
	}

	shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	if (shm_data == MAP_FAILED) {
		perror("nope 2!");
		return NULL;
	}

	struct wl_shm_pool *shm_pool = wl_shm_create_pool(shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(shm_pool, 0,
			width, height, width*4, WL_SHM_FORMAT_ARGB8888);

	memcpy(shm_data, image, size);
	wl_shm_pool_destroy(shm_pool);
	return buffer;
}

int main()
{
	// wl_display represent the connection to the compositor
	display = wl_display_connect(NULL);
	if (display == NULL) {
		perror("Failed to create display!\n");
		return EXIT_FAILURE;
	}
	
	// Get registry
	registry = wl_display_get_registry(display);
	if (registry == NULL) {
		perror("Filed to retrive the registry!\n");
		return EXIT_FAILURE;
	}

	// Add registry listener
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_dispatch(display);
	wl_display_roundtrip(display);

	// Compositor
	if (compositor == NULL || xdg_wm_base == NULL) {
		perror("Something is missing\n");
		return EXIT_FAILURE;
	}

	//Surface
	surface = wl_compositor_create_surface(compositor);
	if (surface == NULL) {
		perror("We have no surface\n");
		return EXIT_FAILURE;
	}

	xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
	xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_toplevel_add_listener(xdg_toplevel, &toplevel_listener, NULL);

	wl_surface_commit(surface);
	wl_display_roundtrip(display);

	struct wl_buffer *buffer = create_buffer();
	if (buffer == NULL) {
		perror("We have no buffer, damn it!");
		return EXIT_FAILURE;
	}

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_commit(surface);

	while (wl_display_dispatch(display) != -1 && running) {
		// Empty
	}

	// Destroy the world
	xdg_toplevel_destroy(xdg_toplevel);
	xdg_surface_destroy(xdg_surface);
	wl_surface_destroy(surface);

	printf("We still alive!\n");
	return EXIT_SUCCESS;
}
