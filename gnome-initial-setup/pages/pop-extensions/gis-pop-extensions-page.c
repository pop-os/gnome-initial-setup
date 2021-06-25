#define PAGE_ID "pop-extensions"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "gis-pop-extensions-page.h"
#include "pop_desktop_widget.h"

struct _GisPopExtensionsPage {
  GisPage parent_instance;
  GtkWidget *header;
  char *title;
};

typedef struct _GisPopExtensionsPage GisPopExtensionsPage;

G_DEFINE_TYPE(GisPopExtensionsPage, gis_pop_extensions_page, GIS_TYPE_PAGE)

static void gis_pop_extensions_page_dispose(GObject *object) {
  GisPopExtensionsPage *priv = gis_pop_extensions_page_get_instance_private(GIS_POP_EXTENSIONS_PAGE(object));
  pop_desktop_widget_string_free (g_steal_pointer (&priv->title));

  G_OBJECT_CLASS(gis_pop_extensions_page_parent_class)->dispose(object);
}

static void gis_pop_extensions_page_locale_changed(GisPage *gis_page) {
  GisPopExtensionsPage *priv = gis_pop_extensions_page_get_instance_private(GIS_POP_EXTENSIONS_PAGE(gis_page));
  pop_desktop_widget_localize ();
  pop_desktop_widget_string_free (g_steal_pointer (&priv->title));
  priv->title = pop_desktop_widget_gis_extensions_title ();
  gis_page_set_title(gis_page,  priv->title);
}

static void gis_pop_extensions_page_realize(GtkWidget *gis_page) {
  GisPopExtensionsPage *page = GIS_POP_EXTENSIONS_PAGE(gis_page);

  GValue icon_show = G_VALUE_INIT;
  g_value_init(&icon_show, G_TYPE_BOOLEAN);
  g_value_set_boolean(&icon_show, FALSE);
  g_object_set_property(G_OBJECT(page->header), "show_icon", &icon_show);

  GTK_WIDGET_CLASS(gis_pop_extensions_page_parent_class)->realize(gis_page);
}

static void gis_pop_extensions_page_class_init(GisPopExtensionsPageClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = gis_pop_extensions_page_dispose;

  GisPageClass *page_class = GIS_PAGE_CLASS (klass);
  page_class->page_id = PAGE_ID;
  page_class->locale_changed = gis_pop_extensions_page_locale_changed;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = gis_pop_extensions_page_realize;
}

static void gis_pop_extensions_page_init(GisPopExtensionsPage *page) {
  GisPopExtensionsPage *priv = gis_pop_extensions_page_get_instance_private(page);
  priv->title = pop_desktop_widget_gis_extensions_title();
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title",  priv->title, NULL);

  GtkWidget *container = pop_desktop_widget_gis_extensions_page(priv->header);

  if (container != NULL) {
    gtk_container_add(GTK_CONTAINER(&priv->parent_instance), container);
    gtk_widget_set_vexpand(GTK_WIDGET(priv), TRUE);
    gtk_widget_show_all(GTK_WIDGET(priv));
    gtk_widget_set_margin_top(GTK_WIDGET(priv), 48);
    gtk_widget_set_margin_bottom(GTK_WIDGET(priv), 48);
    gtk_widget_set_margin_start(GTK_WIDGET(priv), 192);
    gtk_widget_set_margin_end(GTK_WIDGET(priv), 192);
  }

  gis_page_set_complete(GIS_PAGE(page), TRUE);
}

GisPage *gis_prepare_pop_extensions_page(GisDriver *driver) {
  return g_object_new(GIS_TYPE_POP_EXTENSIONS_PAGE, "driver", driver, NULL);
}
