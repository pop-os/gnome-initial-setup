/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2019 System76
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

#ifndef __GIS_ANALYTICS_PAGE_H__
#define __GIS_ANALYTICS_PAGE_H__

#include "gnome-initial-setup.h"

G_BEGIN_DECLS

#define GIS_TYPE_ANALYTICS_PAGE            (gis_analytics_page_get_type ())
#define GIS_ANALYTICS_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIS_TYPE_ANALYTICS_PAGE, GisAnalyticsPage))
#define GIS_ANALYTICS_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GIS_TYPE_ANALYTICS_PAGE, GisAnalyticsPageClass))
#define GIS_IS_ANALYTICS_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIS_TYPE_ANALYTICS_PAGE))
#define GIS_IS_ANALYTICS_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GIS_TYPE_ANALYTICS_PAGE))
#define GIS_ANALYTICS_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GIS_TYPE_ANALYTICS_PAGE, GisAnalyticsPageClass))

typedef struct _GisAnalyticsPage        GisAnalyticsPage;
typedef struct _GisAnalyticsPageClass   GisAnalyticsPageClass;

struct _GisAnalyticsPage
{
  GisPage parent;
};

struct _GisAnalyticsPageClass
{
  GisPageClass parent_class;
};

GType gis_analytics_page_get_type (void);

GisPage *gis_prepare_analytics_page (GisDriver *driver);

G_END_DECLS

#endif /* __GIS_ANALYTICS_PAGE_H__ */
