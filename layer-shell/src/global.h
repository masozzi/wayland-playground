#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>

static int width=715;
static int height=841;

static struct wl_display *display = NULL;
static struct wl_registry *registry = NULL;

static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_surface *surface = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;
static struct zwlr_layer_surface_v1 *layer_surface = NULL;

static struct xdg_wm_base *wm_base = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *toplevel = NULL;

static void noop() { /* empty */ }

static bool window_closed = false;

#endif // GLOBAL_H
