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
 *     Jasper St. Pierre <jstpierre@mecheye.net>
 */

/* Summary page {{{1 */

#define PAGE_ID "summary"

#include "config.h"
#include "summary-resources.h"
#include "gis-summary-page.h"

#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <errno.h>

#include <act/act-user-manager.h>

#include <gdm/gdm-client.h>

#define SERVICE_NAME "gdm-password"

struct _GisSummaryPagePrivate {
  GtkWidget *summary_title;
  GtkWidget *start_button;
  GtkWidget *start_button_label;
  GtkWidget *tagline;
  GtkWidget *left_panel_label;
  GtkWidget *left_panel_image;
  GtkWidget *left_panel_description;
  GtkWidget *right_panel_label;
  GtkWidget *right_panel_description;

  ActUser *user_account;
  const gchar *user_password;
};
typedef struct _GisSummaryPagePrivate GisSummaryPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GisSummaryPage, gis_summary_page, GIS_TYPE_PAGE);

static gboolean
connect_to_gdm (GdmGreeter      **greeter,
                GdmUserVerifier **user_verifier)
{
  GdmClient *client;

  GError *error = NULL;
  gboolean res = FALSE;

  client = gdm_client_new ();

  *greeter = gdm_client_get_greeter_sync (client, NULL, &error);
  if (error != NULL)
    goto out;

  *user_verifier = gdm_client_get_user_verifier_sync (client, NULL, &error);
  if (error != NULL)
    goto out;

  res = TRUE;

 out:
  if (error != NULL) {
    g_warning ("Failed to open connection to GDM: %s", error->message);
    g_error_free (error);
  }

  return res;
}

static void
request_info_query (GisSummaryPage  *page,
                    GdmUserVerifier *user_verifier,
                    const char      *question,
                    gboolean         is_secret)
{
  /* TODO: pop up modal dialog */
  g_debug ("user verifier asks%s question: %s",
           is_secret ? " secret" : "",
           question);
}

static void
on_info (GdmUserVerifier *user_verifier,
         const char      *service_name,
         const char      *info,
         GisSummaryPage  *page)
{
  g_debug ("PAM module info: %s", info);
}

static void
on_problem (GdmUserVerifier *user_verifier,
            const char      *service_name,
            const char      *problem,
            GisSummaryPage  *page)
{
  g_warning ("PAM module error: %s", problem);
}

static void
on_info_query (GdmUserVerifier *user_verifier,
               const char      *service_name,
               const char      *question,
               GisSummaryPage  *page)
{
  request_info_query (page, user_verifier, question, FALSE);
}

static void
on_secret_info_query (GdmUserVerifier *user_verifier,
                      const char      *service_name,
                      const char      *question,
                      GisSummaryPage  *page)
{
  GisSummaryPagePrivate *priv = gis_summary_page_get_instance_private (page);
  gboolean should_send_password = priv->user_password != NULL;

  g_debug ("PAM module secret info query: %s", question);
  if (should_send_password) {
    g_debug ("sending password\n");
    gdm_user_verifier_call_answer_query (user_verifier,
                                         service_name,
                                         priv->user_password,
                                         NULL, NULL, NULL);
    g_clear_pointer (&priv->user_password, (GDestroyNotify) g_free);
  } else {
    request_info_query (page, user_verifier, question, TRUE);
  }
}

static void
on_session_opened (GdmGreeter     *greeter,
                   const char     *service_name,
                   GisSummaryPage *page)
{
  gdm_greeter_call_start_session_when_ready_sync (greeter, service_name,
                                                  TRUE, NULL, NULL);
}

static void
add_uid_file (uid_t uid)
{
  gchar *gis_uid_path;
  gchar *uid_str;
  GError *error = NULL;

  gis_uid_path = g_build_filename (g_get_home_dir (),
                                   "gnome-initial-setup-uid",
                                   NULL);
  uid_str = g_strdup_printf ("%u", uid);

  if (!g_file_set_contents (gis_uid_path, uid_str, -1, &error)) {
      g_warning ("Unable to create %s: %s", gis_uid_path, error->message);
      g_clear_error (&error);
  }

  g_free (uid_str);
  g_free (gis_uid_path);
}

static void
log_user_in (GisSummaryPage *page)
{
  GisSummaryPagePrivate *priv = gis_summary_page_get_instance_private (page);
  GError *error = NULL;
  GdmGreeter *greeter;
  GdmUserVerifier *user_verifier;

  if (!connect_to_gdm (&greeter, &user_verifier)) {
    g_warning ("No GDM connection; not initiating login");
    return;
  }

  g_signal_connect (user_verifier, "info",
                    G_CALLBACK (on_info), page);
  g_signal_connect (user_verifier, "problem",
                    G_CALLBACK (on_problem), page);
  g_signal_connect (user_verifier, "info-query",
                    G_CALLBACK (on_info_query), page);
  g_signal_connect (user_verifier, "secret-info-query",
                    G_CALLBACK (on_secret_info_query), page);

  g_signal_connect (greeter, "session-opened",
                    G_CALLBACK (on_session_opened), page);

  /* We are in NEW_USER mode and we want to make it possible for third
   * parties to find out which user ID we created.
   */
  add_uid_file (act_user_get_uid (priv->user_account));

  gdm_user_verifier_call_begin_verification_for_user_sync (user_verifier,
                                                           SERVICE_NAME,
                                                           act_user_get_user_name (priv->user_account),
                                                           NULL, &error);

  if (error != NULL) {
    g_warning ("Could not begin verification: %s", error->message);
    return;
  }
}

static void
done_cb (GtkButton *button, GisSummaryPage *page)
{
  gis_ensure_stamp_files ();

  switch (gis_driver_get_mode (GIS_PAGE (page)->driver))
    {
    case GIS_DRIVER_MODE_NEW_USER:
      gis_driver_hide_window (GIS_PAGE (page)->driver);
      log_user_in (page);
      break;
    case GIS_DRIVER_MODE_EXISTING_USER:
      g_application_quit (G_APPLICATION (GIS_PAGE (page)->driver));
    default:
      break;
    }
}

static void
gis_summary_page_shown (GisPage *page)
{
  GisSummaryPage *summary = GIS_SUMMARY_PAGE (page);
  GisSummaryPagePrivate *priv = gis_summary_page_get_instance_private (summary);

  gis_driver_save_data (GIS_PAGE (page)->driver);

  gis_driver_get_user_permissions (GIS_PAGE (page)->driver,
                                   &priv->user_account,
                                   &priv->user_password);

  gtk_widget_grab_focus (priv->start_button);
}

static char *
get_item (const char *buffer, const char *name)
{
  char *label, *start, *end, *result;
  char end_char;

  result = NULL;
  start = NULL;
  end = NULL;
  label = g_strconcat (name, "=", NULL);
  if ((start = strstr (buffer, label)) != NULL)
    {
      start += strlen (label);
      end_char = '\n';
      if (*start == '"')
        {
          start++;
          end_char = '"';
        }

      end = strchr (start, end_char);
    }

    if (start != NULL && end != NULL)
      {
        result = g_strndup (start, end - start);
      }

  g_free (label);

  return result;
}

static void
update_distro_name (GisSummaryPage *page)
{
  GisSummaryPagePrivate *priv = gis_summary_page_get_instance_private (page);
  char *buffer;
  char *name_c;
  GString *name;
  gssize name_i;
  gsize name_len;
  char *text;

  name_c = NULL;

  if (g_file_get_contents ("/etc/os-release", &buffer, NULL, NULL))
    {
      name_c = get_item (buffer, "NAME");
      g_free (buffer);
    }

  if (!name_c)
    name_c = g_strdup ("GNOME 3");

  // Copy to a gstring for escaping functions below
  name = g_string_new(name_c);
  g_free(name_c);

  // Escape underscores to allow distributions names to contain underscore
  name_len = strlen(name->str);
  for(name_i = 0; name_i < name_len; name_i++) {
      if (name->str[name_i] == '_') {
          g_string_insert(name, name_i, "_");
          name_i++;
      }
  }

  /* Translators: the parameter here is the name of a distribution,
   * like "Fedora" or "Ubuntu". It falls back to "GNOME 3" if we can't
   * detect any distribution. */
  text = g_strdup_printf (_("_Start Using %s"), name->str);
  gtk_label_set_label (GTK_LABEL (priv->start_button_label), text);
  g_free (text);

  /* Translators: the parameter here is the name of a distribution,
   * like "Fedora" or "Ubuntu". It falls back to "GNOME 3" if we can't
   * detect any distribution. */

  g_string_free (name, TRUE);
}

static char*
freadln (char* path)
{
  FILE *product = fopen (path, "r");
  if (product == NULL) {
      return NULL;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read = getline (&line, &len, product);
  fclose (product);
  return line;
}

static char*
trim (char *str)
{
  char *end;

  while (isspace ((unsigned char)*str)) str++;

  if (*str == 0)
    return str;

  end = str + strlen (str) - 1;
  while(end > str && isspace ((unsigned char)*end)) end--;

  *(end+1) = 0;

  return str;
}

static u_int8_t
has_switchable_graphics ()
{
  char *product_version = freadln ("/sys/class/dmi/id/product_version");
  u_int8_t has_switchable = 0;

  if (product_version) {
    const char* trimmed = trim (product_version);
    has_switchable =
        strcmp (trimmed, "gaze14") == 0 ||
        strcmp (trimmed, "oryp4") == 0 ||
        strcmp (trimmed, "oryp4-b") == 0 ||
        strcmp (trimmed, "oryp5") == 0;
    free (product_version);
  }

  return has_switchable;
}

static char*
get_product_name ()
{
  return freadln ("/sys/class/dmi/id/product_name");
}

static void
gis_summary_page_set_switchable_title (GisSummaryPagePrivate *priv, char *product_name)
{
  char *title_string = g_strdup_printf (_("Your %s Has Switchable Graphics!"), product_name);
  gtk_label_set_label (GTK_LABEL (priv->summary_title), title_string);
  g_free (title_string);
}

static void
gis_summary_page_set_switchable_descriptions (GisSummaryPagePrivate *priv, char *product_name)
{
  char *left_desc = _("Use the system menu on the top panel to switch between "
    "Intel and NVIDIA graphics. Switching will prompt you to restart your "
    "device.");

  char *right_desc = g_strdup_printf (_("To increase battery life, your %s "
    "defaults to Intel graphics. To use external displays, switch to NVIDIA "
    "graphics."), product_name);

  gtk_label_set_line_wrap (GTK_LABEL (priv->left_panel_description), 1);
  gtk_label_set_label (GTK_LABEL (priv->left_panel_description), left_desc);

  gtk_label_set_line_wrap (GTK_LABEL (priv->right_panel_description), 1);
  gtk_label_set_label (GTK_LABEL (priv->right_panel_description), right_desc);

  g_free (right_desc);
}

static void
gis_summary_page_scale_switchable_images (GisSummaryPagePrivate *priv)
{
  GtkImage *left_image = GTK_IMAGE (priv->left_panel_image);
  gint scale = gtk_widget_get_scale_factor (priv->left_panel_image) * 256;

  gtk_image_set_from_pixbuf (
    left_image,
    gdk_pixbuf_scale_simple (
      gtk_image_get_pixbuf (left_image),
      scale,
      scale,
      GDK_INTERP_BILINEAR
    )
  );
}

static void
gis_summary_page_constructed (GObject *object)
{
  GisSummaryPage *page = GIS_SUMMARY_PAGE (object);
  GisSummaryPagePrivate *priv = gis_summary_page_get_instance_private (page);

  G_OBJECT_CLASS (gis_summary_page_parent_class)->constructed (object);

  if (has_switchable_graphics ()) {
    char *product_name = get_product_name ();
    if (product_name) {
        char *trimmed = trim (product_name);
        gis_summary_page_set_switchable_title (priv, trimmed);
        gis_summary_page_set_switchable_descriptions (priv, trimmed);
        free (product_name);
    } else {
        product_name = _("System");
        gis_summary_page_set_switchable_title (priv, product_name);
        gis_summary_page_set_switchable_descriptions (priv, product_name);
    }

    gis_summary_page_scale_switchable_images (priv);
  }

  update_distro_name (page);
  g_signal_connect (priv->start_button, "clicked", G_CALLBACK (done_cb), page);

  gis_page_set_complete (GIS_PAGE (page), TRUE);

  gtk_widget_show (GTK_WIDGET (page));
}

static void
gis_summary_page_locale_changed (GisPage *page)
{
  gis_page_set_title (page, _("Ready to Go"));
  update_distro_name (GIS_SUMMARY_PAGE (page));
}

static void
gis_summary_page_class_init (GisSummaryPageClass *klass)
{
  GisPageClass *page_class = GIS_PAGE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  u_int8_t has_switchable = has_switchable_graphics ();

  char *ui = NULL;
  if (has_switchable) {
    ui = "/org/gnome/initial-setup/gis-summary-page-oryx-switchable.ui";
  } else {
    ui = "/org/gnome/initial-setup/gis-summary-page.ui";
  }

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass), ui);

  if (has_switchable) {
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, summary_title);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, left_panel_label);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, left_panel_image);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, left_panel_description);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, right_panel_label);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, right_panel_description);
  } else {
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, tagline);
  }

  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, start_button);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GisSummaryPage, start_button_label);

  page_class->page_id = PAGE_ID;
  page_class->locale_changed = gis_summary_page_locale_changed;
  page_class->shown = gis_summary_page_shown;
  object_class->constructed = gis_summary_page_constructed;
}

static void
gis_summary_page_init (GisSummaryPage *page)
{
  g_resources_register (summary_get_resource ());

  gtk_widget_init_template (GTK_WIDGET (page));
}

GisPage *
gis_prepare_summary_page (GisDriver *driver)
{
  return g_object_new (GIS_TYPE_SUMMARY_PAGE,
                       "driver", driver,
                       NULL);
}
