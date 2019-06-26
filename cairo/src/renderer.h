#ifndef RENDERER_H
#define RENDERER_H

#include "global.h"

void wp_destroy_cairo(struct wp_status *status);
void wp_init_cairo(struct wp_status *status);
void wp_create_rectangle(struct wp_status *status);

#endif // RENDERER_H
