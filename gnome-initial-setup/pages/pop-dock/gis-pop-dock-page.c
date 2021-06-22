#define PAGE_ID "pop-dock"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "pop-dock-resources.h"
#include "gis-page-header.h"
#include "gis-pop-dock-page.h"

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

static GtkWidget *dock_option_new(GtkWidget *button, char *image_resource) {
  GtkWidget *image = gtk_image_new_from_resource(image_resource);
  gtk_widget_set_halign(image, GTK_ALIGN_START);
  gtk_widget_set_margin_start(image, 4);

  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_container_add(GTK_CONTAINER(container), button);
  gtk_container_add(GTK_CONTAINER(container), image);
  return container;
}

static void gis_pop_dock_page_init(GisPopDockPage *page) {
  g_resources_register(pop_dock_get_resource());

  GisPopDockPage *priv = gis_pop_dock_page_get_instance_private(page);
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title", title(), NULL);

  gis_page_set_title(GIS_PAGE(page), _("Pop Dock"));

  GtkWidget *description = gtk_label_new(_(
    "Continue the desktop setup by choosing your preferred layout."
  ));

  GtkWidget *extra_notice = gtk_label_new(_(
    "Dock appearance, its size, and position can be changed at any time "
    "from the Settings application."
  ));

  GSettings *settings = g_settings_new("org.gnome.shell.extensions.dash-to-dock");

  GtkWidget *check_no_dock = gtk_radio_button_new_with_label(NULL, _("No dock"));
  g_settings_bind(settings, "manualhide", check_no_dock, "active", G_SETTINGS_BIND_DEFAULT);

  GtkWidget *check_extend = gtk_radio_button_new_with_label(NULL, _("Dock extends to edges"));
  gtk_radio_button_join_group(GTK_RADIO_BUTTON(check_extend), GTK_RADIO_BUTTON(check_no_dock));
  g_settings_bind(settings, "extend-height", check_extend, "active", G_SETTINGS_BIND_DEFAULT);

  GtkWidget *check_no_extend = gtk_radio_button_new_with_label(NULL, _("Dock doesn't extend to the edges"));
  gtk_radio_button_join_group(GTK_RADIO_BUTTON(check_no_extend), GTK_RADIO_BUTTON(check_extend));

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_extend), TRUE);

  GtkWidget *dock_selector = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_vexpand(dock_selector, TRUE);
  gtk_widget_set_valign(dock_selector, GTK_ALIGN_CENTER);
  gtk_box_set_homogeneous(GTK_BOX(dock_selector), TRUE);
  gtk_container_add(GTK_CONTAINER(dock_selector), dock_option_new(check_no_dock, "/org/gnome/initial-setup/pop-dock-none.png"));
  gtk_container_add(GTK_CONTAINER(dock_selector), dock_option_new(check_extend, "/org/gnome/initial-setup/pop-dock-extends.png"));
  gtk_container_add(GTK_CONTAINER(dock_selector), dock_option_new(check_no_extend, "/org/gnome/initial-setup/pop-dock-dynamic.png"));

  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(container, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(container), priv->header);
  gtk_container_add(GTK_CONTAINER(container), description);
  gtk_container_add(GTK_CONTAINER(container), dock_selector);
  gtk_container_add(GTK_CONTAINER(container), extra_notice);

  gtk_container_add(GTK_CONTAINER(&priv->parent_instance), container);
  gtk_widget_set_vexpand(GTK_WIDGET(priv), TRUE);
  gtk_widget_show_all(GTK_WIDGET(priv));
  gtk_widget_set_margin_top(GTK_WIDGET(priv), 48);
  gtk_widget_set_margin_bottom(GTK_WIDGET(priv), 48);

  gis_page_set_complete(GIS_PAGE(page), TRUE);
}

GisPage *gis_prepare_pop_dock_page(GisDriver *driver) {
  return g_object_new(GIS_TYPE_POP_DOCK_PAGE, "driver", driver, NULL);
}
