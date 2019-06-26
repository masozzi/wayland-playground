#ifndef WAYLAND_H
#define WAYLAND_H

#include <stdbool.h>

#include <wayland-client.h>

void wp_init_wayland(struct wp_status *status);
void wp_create_buffer(struct wp_status *status);

#endif // WAYLAND_H
