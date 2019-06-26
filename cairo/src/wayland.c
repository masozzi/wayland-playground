#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include "global.h"
#include "shm.h"
#include "wayland.h"
#include "xdg-shell-client-protocol.h"

//
// Handlers
//
static void noop()
{
	// Yes, it's empty!
}

static void registry_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	struct wp_status *status = data;

	printf("we got %s\n", interface);
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		status->compositor = wl_registry_bind(wl_registry, name,
				&wl_compositor_interface, version);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		status->xdg_wm_base = wl_registry_bind(wl_registry, name,
				&xdg_wm_base_interface, version);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		printf("\tbinding %s\n", interface);
		status->shm = wl_registry_bind(wl_registry, name,
				&wl_shm_interface, version);
	}
}

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial)
{
	xdg_surface_ack_configure(xdg_surface, serial);
}

static void xdg_toplevel_handle_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t width,
		int32_t height, struct wl_array *states)
{
	struct wp_status *status = data;

	printf(":: Config has changed\n");
	printf("\t:: width: %d\n\t:: hight: %d\n", width, height);

	status->width = width;
	status->height = height;
	status->new_dimension = true;
}

static void xdg_toplevel_handle_close(void *data,
		struct xdg_toplevel *xdg_toplevel)
{
	struct wp_status *status = data;

	printf(":: Application closing\n");
	status->running = false;
}

//
// Listeners
//
static struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = noop
};

static struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure
};

static struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close
};

//
// Other
//
static void init_display(struct wp_status *status)
{
	struct wl_display *tmp = wl_display_connect(NULL);
	if (tmp == NULL) {
		fprintf(stderr, "-- Cannot obtain display!");
		exit(EXIT_FAILURE);
	}

	status->display = tmp;
}

static void init_registry(struct wp_status *status)
{
	struct wl_registry *tmp = wl_display_get_registry(status->display);
	if (tmp == NULL) {
		fprintf(stderr, "-- Failed to obtain registry");
		exit(EXIT_FAILURE);
	}

	status->registry = tmp;
	wl_registry_add_listener(tmp, &registry_listener, status);

	wl_display_dispatch(status->display);
	wl_display_roundtrip(status->display);

}

static void init_surface(struct wp_status *status)
{
	if (status->compositor == NULL) {
		fprintf(stderr, "-- Cannot create surface, missing compositor\n");
		exit(EXIT_FAILURE);
	}

	struct wl_surface *tmp = wl_compositor_create_surface(status->compositor);
	if (tmp == NULL) {
		fprintf(stderr, "-- Cannot create surface");
		exit(EXIT_FAILURE);
	}

	status->surface = tmp;
}

static void init_xdg_surface(struct wp_status *status)
{
	if (status->xdg_wm_base == NULL) {
		fprintf(stderr, "-- Cannot create xdg surface, missing xdg wm base\n");
		exit(EXIT_FAILURE);
	}

	struct xdg_surface *tmp = xdg_wm_base_get_xdg_surface(
			status->xdg_wm_base, status->surface);
	if (tmp == NULL) {
		fprintf(stderr, "-- Cannot create xdg surface");
		exit(EXIT_FAILURE);
	}

	status->xdg_surface = tmp;
	xdg_surface_add_listener(status->xdg_surface,
			&xdg_surface_listener, status);
}

static void init_xdg_toplevel(struct wp_status *status)
{
	struct xdg_toplevel *tmp = xdg_surface_get_toplevel(status->xdg_surface);
	if (tmp == NULL) {
		fprintf(stderr, "-- Cannot create xdg toplevel");
		exit(EXIT_FAILURE);
	}

	status->xdg_toplevel = tmp;
	xdg_toplevel_add_listener(tmp, &xdg_toplevel_listener, status);
}

//
// Public methods
//
void wp_init_wayland(struct wp_status *status)
{
	printf(":: Init wayland\n");
	init_display(status);
	init_registry(status);
	init_surface(status);
	init_xdg_surface(status);
	init_xdg_toplevel(status);

	wl_surface_commit(status->surface);
	wl_display_roundtrip(status->display);
}

static struct wl_buffer *create_buffer(struct wp_status *status)
{
	int stride = status->width * 4;
	off_t size = stride * status->height;
	printf("\t-- width: %d\n", status->width);
	printf("\t-- height: %d\n", status->height);
	printf("\t-- size: %ld\n", size);

	int fd = create_shm_file(size);
	void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, 0);
        if (shm_data == MAP_FAILED) {
                fprintf(stderr, "!! Could not create shm!\n");
                close(fd);
		exit(EXIT_FAILURE);
        }

        struct wl_shm_pool *shm_pool = wl_shm_create_pool(status->shm, fd, size);
        struct wl_buffer *buffer = wl_shm_pool_create_buffer(shm_pool, 0,
        		status->width, status->height, stride,
        		WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy(shm_pool);
        close(fd);

        status->data = shm_data;
        status->size = size;
        return buffer;
}

void wp_create_buffer(struct wp_status *status)
{
	printf(":: Creating buffer\n");
	if (status->width == 0 || status->height == 0) {
		status->width = 800;
		status->height = 600;
	}
	
	status->buffer = create_buffer(status);
}
