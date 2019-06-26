#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

#include <cairo/cairo.h>

#include "cairo.h"
#include "global.h"

static void init_surface(struct wp_status *status)
{

	cairo_surface_t *surface;
	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, status->width);

	surface = cairo_image_surface_create_for_data(status->data,
			CAIRO_FORMAT_ARGB32, status->width, status->height, stride);
	if (surface == NULL) {
		fprintf(stderr, "!! Could not create cairo surface");
		exit(EXIT_FAILURE);
	}

	status->c_surface = surface;
}

static void init_context(struct wp_status *status)
{
	cairo_t *context = cairo_create(status->c_surface);
	if (context == NULL) {
		fprintf(stderr, "!! Could not create cairo context");
		exit(EXIT_FAILURE);
	}

	status->c_context = context;
}

static void create_background(struct wp_status *status)
{
	cairo_t *ctx = status->c_context;

	cairo_set_source_rgb(ctx, 0, 0, 0);
	cairo_rectangle(ctx, 0, 0, status->width, status->height);
	cairo_fill(ctx);
}

//
// Public methods
//
void wp_destroy_cairo(struct wp_status *status)
{
	cairo_destroy(status->c_context);
	status->c_context = NULL;

	cairo_surface_destroy(status->c_surface);
	status->c_surface = NULL;
}

void wp_init_cairo(struct wp_status *status)
{
	init_surface(status);
	init_context(status);
}

void wp_create_rectangle(struct wp_status *status)
{
	printf(":: Creating rectangle\n");
	printf("\t-- new dim? %d\n", status->new_dimension);
	printf("\t-- width %d\n", status->width);
	printf("\t-- height %d\n", status->height);
	if (status->new_dimension) {
		printf("\t:: new dimensions\n");
		wp_destroy_cairo(status);
		wp_init_cairo(status);

		status->new_dimension = false;
	}

	create_background(status);
}
