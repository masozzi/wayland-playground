#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/types.h>

#include "global.h"
#include "renderer.h"
#include "wayland.h"

static void init_status(struct wp_status *status)
{
	status->running = true;

	status->width = 0;
	status->height = 0;
	status->new_dimension = false;

	status->display = NULL;
	status->registry = NULL;
	status->compositor = NULL;
	status->surface = NULL;
	status->xdg_wm_base = NULL;
	status->xdg_surface = NULL;
	status->xdg_toplevel = NULL;
	status->shm = NULL;
	status->buffer = NULL;

	status->c_surface = NULL;
	status->c_context = NULL;

	status->data = 0;
}

static void run_loop(struct wp_status *status)
{
	int frame = 0;
	assert(status->display);

	clock_t start_time = clock();
	clock_t end_time = start_time;

	while(status->running) {
		if ((end_time - start_time) / 1000 < 16) {
			end_time = clock();
			continue;
		}
		start_time = clock();

		printf("\n-- Frame %d\n", frame);
		frame++;

		wp_create_buffer(status);
		wp_create_rectangle(status);

		wl_surface_attach(status->surface, status->buffer, 0, 0);
		wl_surface_commit(status->surface);

		// create cairo surface with the share memory
		// who knows
		int err = wl_display_dispatch(status->display);
		if (err < 0) {
			fprintf(stderr, "!! Error while dispatching: %m\n");
			break;
		}

		// Destroy buffer and shm data
		munmap(status->data, status->size);
		status->data = NULL;

		wl_buffer_destroy(status->buffer);
		status->buffer = NULL;

		end_time = clock();
	}

	// kill everybody and say goodbye
}

int main(void)
{
	struct wp_status status = {};
	init_status(&status);

	wp_init_wayland(&status);
	wp_init_cairo(&status);
	run_loop(&status);
	
	return 0;
}
