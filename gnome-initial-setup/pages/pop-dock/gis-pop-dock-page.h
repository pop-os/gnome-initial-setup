#pragma once

#include <glib.h>
#include <glib-object.h>

#include "gnome-initial-setup.h"

G_BEGIN_DECLS

#define GIS_TYPE_POP_DOCK_PAGE (gis_pop_dock_page_get_type ())
G_DECLARE_FINAL_TYPE (GisPopDockPage, gis_pop_dock_page, GIS, POP_DOCK_PAGE, GisPage)

GType gis_pop_dock_page_get_type (void);

GisPage *gis_prepare_pop_dock_page (GisDriver *driver);

G_END_DECLS
