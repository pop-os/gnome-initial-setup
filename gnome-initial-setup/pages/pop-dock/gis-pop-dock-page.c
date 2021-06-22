#define PAGE_ID "pop-dock"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "gis-page-header.h"
#include "gis-pop-dock-page.h"
#include "pop_desktop_widget.h"

struct _GisPopDockPage {
  GisPage parent_instance;
  GtkWidget *header;
};

typedef struct _GisPopDockPage GisPopDockPage;

G_DEFINE_TYPE(GisPopDockPage, gis_pop_dock_page, GIS_TYPE_PAGE)

static char *title() {
  return _("Welcome to Pop!_OS");
}

static void gis_pop_dock_page_dispose(GObject *object) {
  G_OBJECT_CLASS(gis_pop_dock_page_parent_class)->dispose(object);
}

static void gis_pop_dock_page_locale_changed(GisPage *gis_page) {
  gis_page_set_title(gis_page, title());
}

static void gis_pop_dock_page_realize(GtkWidget *gis_page) {
  GisPopDockPage *page = GIS_POP_DOCK_PAGE(gis_page);

  GValue icon_show = G_VALUE_INIT;
  g_value_init(&icon_show, G_TYPE_BOOLEAN);
  g_value_set_boolean(&icon_show, FALSE);
  g_object_set_property(G_OBJECT(page->header), "show_icon", &icon_show);

  GTK_WIDGET_CLASS(gis_pop_dock_page_parent_class)->realize(gis_page);
}

static void gis_pop_dock_page_class_init(GisPopDockPageClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = gis_pop_dock_page_dispose;

  GisPageClass *page_class = GIS_PAGE_CLASS (klass);
  page_class->page_id = PAGE_ID;
  page_class->locale_changed = gis_pop_dock_page_locale_changed;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = gis_pop_dock_page_realize;
}

static void gis_pop_dock_page_init(GisPopDockPage *page) {
  GisPopDockPage *priv = gis_pop_dock_page_get_instance_private(page);
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title", title(), NULL);

  gis_page_set_title(GIS_PAGE(page), _("Pop Dock"));

  gtk_container_add(GTK_CONTAINER(&priv->parent_instance), pop_desktop_widget_gis_dock_page(priv->header));
  gtk_widget_set_vexpand(GTK_WIDGET(priv), TRUE);
  gtk_widget_show_all(GTK_WIDGET(priv));
  gtk_widget_set_margin_top(GTK_WIDGET(priv), 48);
  gtk_widget_set_margin_bottom(GTK_WIDGET(priv), 48);

  gis_page_set_complete(GIS_PAGE(page), TRUE);
}

GisPage *gis_prepare_pop_dock_page(GisDriver *driver) {
  return g_object_new(GIS_TYPE_POP_DOCK_PAGE, "driver", driver, NULL);
}
