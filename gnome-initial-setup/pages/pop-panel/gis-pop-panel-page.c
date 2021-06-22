#define PAGE_ID "pop-panel"

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "gis-page-header.h"
#include "gis-pop-panel-page.h"

struct _GisPopPanelPage {
  GisPage parent_instance;
  GtkWidget *header;
};

typedef struct _GisPopPanelPage GisPopPanelPage;

G_DEFINE_TYPE(GisPopPanelPage, gis_pop_panel_page, GIS_TYPE_PAGE)

static char *title() {
  return _("Configure the Top Bar");
}

static void gis_pop_panel_page_dispose(GObject *object) {
  G_OBJECT_CLASS(gis_pop_panel_page_parent_class)->dispose(object);
}

static void gis_pop_panel_page_locale_changed(GisPage *gis_page) {
  gis_page_set_title(gis_page, title());
}

static void gis_pop_panel_page_realize(GtkWidget *gis_page) {
  GisPopPanelPage *page = GIS_POP_PANEL_PAGE(gis_page);

  GValue icon_show = G_VALUE_INIT;
  g_value_init(&icon_show, G_TYPE_BOOLEAN);
  g_value_set_boolean(&icon_show, FALSE);
  g_object_set_property(G_OBJECT(page->header), "show_icon", &icon_show);

  GTK_WIDGET_CLASS(gis_pop_panel_page_parent_class)->realize(gis_page);
}

static void gis_pop_panel_page_class_init(GisPopPanelPageClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = gis_pop_panel_page_dispose;

  GisPageClass *page_class = GIS_PAGE_CLASS (klass);
  page_class->page_id = PAGE_ID;
  page_class->locale_changed = gis_pop_panel_page_locale_changed;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = gis_pop_panel_page_realize;
}

/** Create a row of the settings list box */
GtkWidget *settings_row(GtkWidget *widget, char *label) {
  GtkWidget *description = gtk_label_new(label);
  gtk_widget_set_hexpand(description, TRUE);
  gtk_label_set_xalign(GTK_LABEL(description), 0.0);

  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_widget_set_margin_start(container, 12);
  gtk_widget_set_margin_top(container, 6);
  gtk_widget_set_margin_end(container, 12);
  gtk_widget_set_margin_bottom(container, 6);
  gtk_container_add(GTK_CONTAINER(container), description);
  gtk_container_add(GTK_CONTAINER(container), widget);

  return container;
}

static void panel_position_changed(GtkComboBox *combo, GSettings *settings) {
  g_settings_set_boolean(settings, "show-panel", gtk_combo_box_get_active(combo) == 1);
}

/** Chooses whether the panel should show on all displays, or only the primary */
GtkWidget *panel_position_row(GSettings *settings) {
  GtkWidget *combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("Primary Display"));
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("All Displays"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), g_settings_get_boolean(settings, "show-panel") ? 1 : 0);
  g_signal_connect(GTK_COMBO_BOX(combo), "changed", G_CALLBACK(panel_position_changed), settings);

  return settings_row(combo, _("Show Top Bar on Display"));
}

static void date_position_changed(GtkComboBox *combo, GSettings *settings) {
  gint active = gtk_combo_box_get_active(combo);
  if (active == -1) active = 0;

  g_settings_set_enum(settings, "clock-alignment", active);
}

/** Controls the positioning of the notification area */
GtkWidget *date_position_row(GSettings *settings) {
  GtkWidget *combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("Center"));
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("Left"));
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), _("Right"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), g_settings_get_enum(settings, "clock-alignment"));
  g_signal_connect(GTK_COMBO_BOX(combo), "changed", G_CALLBACK(date_position_changed), settings);

  return settings_row(combo, _("Date & Time and Notifications Position"));
}

static void list_header(GtkListBoxRow *row, GtkListBoxRow *before, gpointer data) {
  if (before != NULL) gtk_list_box_row_set_header(row, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
}

static void gis_pop_panel_page_init(GisPopPanelPage *page) {
  gis_page_set_title(GIS_PAGE(page), _("Pop Panel"));

  GisPopPanelPage *priv = gis_pop_panel_page_get_instance_private(page);
  priv->header = g_object_new(GIS_TYPE_PAGE_HEADER, "title", title(), NULL);

  GtkWidget *extra_notice = gtk_label_new(_(
    "Top bar configuration can be changed at any time from the Settings application."
  ));

  GSettings *cosmic_settings = g_settings_new("org.gnome.shell.extensions.pop-cosmic");
  GSettings *multi_monitor_settings = g_settings_new("org.gnome.shell.extensions.multi-monitors-add-on");

  GtkWidget *show_applications_toggle = gtk_switch_new();
  g_settings_bind(cosmic_settings, "show-applications-button", show_applications_toggle, "active", G_SETTINGS_BIND_DEFAULT);

  GtkWidget *show_workspaces_toggle = gtk_switch_new();
  g_settings_bind(cosmic_settings, "show-workspaces-button", show_workspaces_toggle, "active", G_SETTINGS_BIND_DEFAULT);

  GtkWidget *settings_box = gtk_list_box_new();
  gtk_list_box_set_header_func(GTK_LIST_BOX(settings_box), list_header, NULL, NULL);

  gtk_container_add(GTK_CONTAINER(settings_box), settings_row(show_applications_toggle, _("Show Applications Button")));
  gtk_container_add(GTK_CONTAINER(settings_box), settings_row(show_workspaces_toggle, _("Show Workspaces Button")));
  gtk_container_add(GTK_CONTAINER(settings_box), panel_position_row(multi_monitor_settings));
  gtk_container_add(GTK_CONTAINER(settings_box), date_position_row(cosmic_settings));

  GtkWidget *framed_box = gtk_frame_new(NULL);
  gtk_widget_set_margin_top(framed_box, 32);
  gtk_widget_set_vexpand(framed_box, TRUE);
  gtk_widget_set_valign(framed_box, GTK_ALIGN_START);
  gtk_container_add(GTK_CONTAINER(framed_box), settings_box);

  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(container, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(container), priv->header);
  gtk_container_add(GTK_CONTAINER(container), framed_box);
  gtk_container_add(GTK_CONTAINER(container), extra_notice);

  gtk_container_add(GTK_CONTAINER(&priv->parent_instance), container);
  gtk_widget_show_all(GTK_WIDGET(priv));
  gtk_widget_set_vexpand(GTK_WIDGET(priv), TRUE);
  gtk_widget_set_margin_top(GTK_WIDGET(priv), 48);
  gtk_widget_set_margin_bottom(GTK_WIDGET(priv), 48);

  gis_page_set_complete (GIS_PAGE (page), TRUE);
}

GisPage *gis_prepare_pop_panel_page(GisDriver *driver) {
  return g_object_new(GIS_TYPE_POP_PANEL_PAGE, "driver", driver, NULL);
}
