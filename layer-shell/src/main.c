#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>

#include "global.h"
#include "listeners.h"
#include "shm.h"
#include "image.h"

static void init_display()
{
	display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Cannot connect to wayland display");
		exit(EXIT_FAILURE);
	}
}

static void init_registry()
{
	registry = wl_display_get_registry(display);
	if (registry == NULL) {
		fprintf(stderr, "Cannot get registry");
	}

	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_dispatch(display);
	wl_display_roundtrip(display);
}

static void init_surface()
{
	if (compositor == NULL) {
		fprintf(stderr, "No compasitor dude!");
	}

	surface = wl_compositor_create_surface(compositor);
}

static void init_layer()
{
	if (layer_shell == NULL || surface == NULL) {
		fprintf(stderr, "Sorry, no layer shell budy!\n");
	}

	uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
	layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
			surface, NULL, layer, "Hello");
	zwlr_layer_surface_v1_add_listener(layer_surface,
			&layer_surface_listener, NULL);

	uint32_t anchor = 0;
	zwlr_layer_surface_v1_set_size(layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
}

static struct wl_buffer *create_buffer()
{
	int stride = width*4;
	off_t size = stride * height;

	int fd = create_shm_file(size);
	if (fd < 0) {
		fprintf(stderr, "No file descriptor baby!");
	}

	void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, 0);
	if (shm_data == MAP_FAILED) {
		fprintf(stderr, "No shm data baby!");
		close(fd);
		return NULL;
	}

	struct wl_shm_pool *shm_pool = wl_shm_create_pool(shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(shm_pool, 0, width,
			height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(shm_pool);

	memcpy(shm_data, image, size);
	return buffer;
}

static void destroy()
{
	wl_surface_destroy(surface);
}

int main(void)
{
	init_display();
	init_registry();
	init_surface();
	init_layer();
	struct wl_buffer *buffer = create_buffer();
	if (buffer == NULL) {
		fprintf(stderr, "We have no buffer, ouch!");
	}

	wl_surface_commit(surface);
	wl_display_roundtrip(display);

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_damage_buffer(surface, 0, 0, INT32_MAX, INT32_MAX);
	wl_surface_commit(surface);

	while(wl_display_dispatch(display) != -1 && !window_closed) {
		printf("We in boys!\n");
		// Empty
	}

	wl_buffer_destroy(buffer);
	destroy();

	return 0;
}
