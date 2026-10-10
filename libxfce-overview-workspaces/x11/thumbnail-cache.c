/*
 * thumbnail-cache: On-disk frozen cache of live window thumbnails
 *
 * See thumbnail-cache.h for details.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/x11/thumbnail-cache.h>

#include <errno.h>
#include <glib/gstdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


/* Ensure cache directory exists with mode 0700 */
void xfce_overview_workspaces_thumbnail_cache_ensure_dir_0700(void)
{
	/* g_mkdir_with_parents is a no-op if directory already exists;
	 * always chmod to enforce 0700 even if it pre-existed.
	 */
	g_mkdir_with_parents(XFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE_DIR, 0700);
	chmod(XFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE_DIR, 0700);
}

/* Build "/tmp/xfce-overview-workspaces/win-<XID>.jpg". Caller frees with g_free(). */
char* xfce_overview_workspaces_thumbnail_cache_path_for_xid(gulong xid)
{
	return(g_strdup_printf("%s/win-%lu.jpg",
							XFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE_DIR,
							(unsigned long)xid));
}

/* Save pixbuf as JPEG quality 75 with atomic rename. Returns TRUE on success. */
gboolean xfce_overview_workspaces_thumbnail_cache_save_pixbuf(GdkPixbuf *pix, gulong xid)
{
	char		*path;
	char		*tmpPath;
	GError		*error;
	gboolean	saved;

	if(!pix || xid==0) return(FALSE);

	xfce_overview_workspaces_thumbnail_cache_ensure_dir_0700();

	path=xfce_overview_workspaces_thumbnail_cache_path_for_xid(xid);
	tmpPath=g_strdup_printf("%s.tmp.%d", path, (int)getpid());

	error=NULL;
	saved=gdk_pixbuf_save(pix, tmpPath, "jpeg", &error, "quality", "75", NULL);
	if(!saved)
	{
		g_warning("Could not save frozen thumbnail for XID %lu to '%s': %s",
					(unsigned long)xid,
					tmpPath,
					(error && error->message) ? error->message : "Unknown error");
		if(error) g_error_free(error);
		g_unlink(tmpPath);
		g_free(tmpPath);
		g_free(path);
		return(FALSE);
	}

	if(rename(tmpPath, path)!=0)
	{
		g_warning("Could not rename frozen thumbnail '%s' to '%s': %s",
					tmpPath, path, g_strerror(errno));
		g_unlink(tmpPath);
		g_free(tmpPath);
		g_free(path);
		return(FALSE);
	}

	chmod(path, 0600);

	g_free(tmpPath);
	g_free(path);
	return(TRUE);
}

/* Remove cached file for XID (best effort). */
void xfce_overview_workspaces_thumbnail_cache_invalidate_xid(gulong xid)
{
	char		*path;

	if(xid==0) return;

	path=xfce_overview_workspaces_thumbnail_cache_path_for_xid(xid);
	g_unlink(path);
	g_free(path);
}
