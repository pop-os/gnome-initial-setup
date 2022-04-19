/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2012 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Written by:
 *     System76 <info@system76.com>
 */

#define PAGE_ID "analytics"

#include "config.h"
#include "gis-analytics-page.h"
#include "gis-assistant.h"
#include "pop_analytics_panel.h"
#include "pop_theme_switcher.h"

struct _GisAnalyticsPagePrivate {};

typedef struct _GisAnalyticsPagePrivate GisAnalyticsPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GisAnalyticsPage, gis_analytics_page, GIS_TYPE_PAGE);

static void analytics_result(void * data, bool agreed) {
    GisAnalyticsPage *page = GIS_ANALYTICS_PAGE (data);

    gis_assistant_next_page (gis_driver_get_assistant (GIS_PAGE (page)->driver));
}

static void gis_analytics_page_locale_changed (GisPage *page) {
  gis_page_set_title (GIS_PAGE (page), _("Analytics"));
}

static void gis_analytics_page_constructed (GObject *object) {
    GisAnalyticsPage *page = GIS_ANALYTICS_PAGE (object);
    GisAnalyticsPagePrivate *priv = gis_analytics_page_get_instance_private (page);

    G_OBJECT_CLASS (gis_analytics_page_parent_class)->constructed (object);

    GtkContainer *container = GTK_CONTAINER (page);

    GtkWidget *widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (widget, GTK_ALIGN_START);
    gtk_container_add (container, widget);

    gtk_widget_show_all (GTK_WIDGET (container));

    gis_page_set_complete (GIS_PAGE (page), false);
    gis_page_set_needs_accept (GIS_PAGE (page), true);

    if (pop_analytics_panel_should_show ()) {
        // Add analytics, consumes widget
        pop_analytics_panel_initial_setup_attach (GTK_CONTAINER (widget), analytics_result, (void *)page);
    } else {
        // Skip page
        gtk_widget_set_visible (GTK_WIDGET (page), FALSE);
    }
}

static void gis_analytics_page_class_init (GisAnalyticsPageClass *klass) {
    GisPageClass *page_class = GIS_PAGE_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    page_class->page_id = PAGE_ID;
    page_class->locale_changed = gis_analytics_page_locale_changed;

    object_class->constructed = gis_analytics_page_constructed;
}

static void gis_analytics_page_init (GisAnalyticsPage *page) {
    pop_analytics_panel_init ();
}

GisPage * gis_prepare_analytics_page (GisDriver *driver) {
    return g_object_new (GIS_TYPE_ANALYTICS_PAGE, "driver", driver, NULL);
}
