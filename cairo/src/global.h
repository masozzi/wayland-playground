#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include <sys/types.h>

#include <cairo/cairo.h>

struct wp_status
{
	bool running;

	int32_t width;
	int32_t height;
	bool new_dimension;

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_surface *surface;
	struct xdg_wm_base *xdg_wm_base;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	struct wl_shm *shm;
	struct wl_buffer *buffer;

	cairo_surface_t *c_surface;
	cairo_t *c_context;

	// shared memory
	void *data;
	off_t size;
};

#endif // GLOBAL_H
