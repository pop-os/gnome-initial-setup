#define PAGE_ID "pop-gesture"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "gis-pop-gesture-page.h"
#include "pop_desktop_widget.h"

struct _GisPopGesturePage {
  GisPage parent_instance;
  GtkWidget *header;
  char *title;
};

typedef struct _GisPopGesturePage GisPopGesturePage;

G_DEFINE_TYPE(GisPopGesturePage, gis_pop_gesture_page, GIS_TYPE_PAGE)

static void gis_pop_gesture_page_dispose(GObject *object) {
  GisPopGesturePage *priv = gis_pop_gesture_page_get_instance_private(GIS_POP_GESTURE_PAGE(object));
  pop_desktop_widget_string_free (g_steal_pointer (&priv->title));

  G_OBJECT_CLASS(gis_pop_gesture_page_parent_class)->dispose(object);
}

static void gis_pop_gesture_page_locale_changed(GisPage *gis_page) {
  GisPopGesturePage *priv = gis_pop_gesture_page_get_instance_private(GIS_POP_GESTURE_PAGE(gis_page));
  pop_desktop_widget_localize ();
  pop_desktop_widget_string_free (g_steal_pointer (&priv->title));
  priv->title = pop_desktop_widget_gis_gestures_title ();
  gis_page_set_title(gis_page,  priv->title);
}

static void gis_pop_gesture_page_realize(GtkWidget *gis_page) {
  GisPopGesturePage *page = GIS_POP_GESTURE_PAGE(gis_page);

  GValue icon_show = G_VALUE_INIT;
  g_value_init(&icon_show, G_TYPE_BOOLEAN);
  g_value_set_boolean(&icon_show, FALSE);
  g_object_set_property(G_OBJECT(page->header), "show_icon", &icon_show);

  GTK_WIDGET_CLASS(gis_pop_gesture_page_parent_class)->realize(gis_page);
}

static void gis_pop_gesture_page_class_init(GisPopGesturePageClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = gis_pop_gesture_page_dispose;

  GisPageClass *page_class = GIS_PAGE_CLASS (klass);
  page_class->page_id = PAGE_ID;
  page_class->locale_changed = gis_pop_gesture_page_locale_changed;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = gis_pop_gesture_page_realize;
}

static void gis_pop_gesture_page_init(GisPopGesturePage *page) {
  GisPopGesturePage *priv = gis_pop_gesture_page_get_instance_private(page);
  priv->title = pop_desktop_widget_gis_gestures_title();
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title",  priv->title, NULL);

  gtk_container_add(GTK_CONTAINER(&priv->parent_instance), pop_desktop_widget_gis_gestures_page(priv->header));
  gtk_widget_set_vexpand(GTK_WIDGET(priv), TRUE);
  gtk_widget_show_all(GTK_WIDGET(priv));
  gtk_widget_set_margin_top(GTK_WIDGET(priv), 48);
  gtk_widget_set_margin_bottom(GTK_WIDGET(priv), 48);
  gtk_widget_set_margin_start(GTK_WIDGET(priv), 192);
  gtk_widget_set_margin_end(GTK_WIDGET(priv), 192);

  gis_page_set_complete(GIS_PAGE(page), TRUE);
}

GisPage *gis_prepare_pop_gesture_page(GisDriver *driver) {
  return g_object_new(GIS_TYPE_POP_GESTURE_PAGE, "driver", driver, NULL);
}
