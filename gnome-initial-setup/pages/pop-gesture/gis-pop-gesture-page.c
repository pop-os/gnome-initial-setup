#define PAGE_ID "pop-gesture"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "pop-gesture-resources.h"
#include "gis-page-header.h"
#include "gis-pop-gesture-page.h"

struct _GisPopGesturePage {
  GisPage parent_instance;
  GtkWidget *header;
};

typedef struct _GisPopGesturePage GisPopGesturePage;

G_DEFINE_TYPE(GisPopGesturePage, gis_pop_gesture_page, GIS_TYPE_PAGE)

static char *title() {
  return _("Use Gestures for Easier Navigation");
}

static void gis_pop_gesture_page_dispose(GObject *object) {
  G_OBJECT_CLASS(gis_pop_gesture_page_parent_class)->dispose(object);
}

static void gis_pop_gesture_page_locale_changed(GisPage *gis_page) {
  gis_page_set_title(gis_page, title());
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
  g_resources_register(pop_gesture_get_resource());

  GisPopGesturePage *priv = gis_pop_gesture_page_get_instance_private(page);
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title", title(), NULL);

  gis_page_set_title(GIS_PAGE(page), _("Pop Gesture"));

  GtkWidget *description = gtk_label_new(_(
    "Use four finger swip left to pen Workspaces and windows overview, four fingers swipe "
    "right to pen Applications, and four fingers swipe up or down to switch between "
    "workspaces. Swipe with three fingers to switch between windows."
  ));

  gtk_label_set_line_wrap(GTK_LABEL(description), TRUE);
  gtk_label_set_justify(GTK_LABEL(description), GTK_JUSTIFY_CENTER);

  GtkWidget *image = gtk_image_new_from_resource("/org/gnome/initial-setup/pop-gesture.png");
  gtk_widget_set_vexpand(image, TRUE);
  gtk_widget_set_halign(image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(image, GTK_ALIGN_START);
  gtk_widget_set_margin_top(image, 32);

  GtkWidget *extra_notice = gtk_label_new(_(
    "Super key configuration can be changed at any time from the Settings application."
  ));

  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(container, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(container), priv->header);
  gtk_container_add(GTK_CONTAINER(container), description);
  gtk_container_add(GTK_CONTAINER(container), image);
  gtk_container_add(GTK_CONTAINER(container), extra_notice);

  gtk_container_add(GTK_CONTAINER(&priv->parent_instance), container);
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
