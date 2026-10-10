#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define COGL_ENABLE_EXPERIMENTAL_API

#include <libxfdashboard/x11/window-content-frozen.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libxfdashboard/x11/thumbnail-cache.h>
#include <libxfdashboard/x11/window-tracker-window-x11.h>


void xfdashboard_window_content_frozen_pixbuf_free_data(guchar *inPixels, gpointer inUserData G_GNUC_UNUSED)
{
	g_free(inPixels);
}

void xfdashboard_window_content_frozen_save(gboolean isFallback,
											CoglTexture *texture,
											Window xWindowID,
											XfdashboardWindowTrackerWindowX11 *window,
											Window *inoutFrozenCacheXID)
{
	CoglTextureComponents					textureComponents;
	CoglPixelFormat							textureFormat;
	gboolean								hasAlpha;
	guint									textureWidth;
	guint									textureHeight;
	gint									textureSize;
	gint									gotSize;
	guint8									*textureData;
	GdkPixbuf								*pixbuf;
	Window									frozenCacheXID;

	/* Never save fallback (icon) frames */
	if(isFallback) return;
	if(!texture) return;
	if(xWindowID==None) return;

	/* NOTE: key the on-disk cache by CLIENT xid (stable, matches the loader
	 * lookup via wnck xid). xWindowID may be the FRAME window when
	 * the 'include-window-frame' style is set (our themes set it), which
	 * would never match the loader and would orphan files.
	 */
	{
		gulong						clientXID;

		clientXID=0;
		if(window)
		{
			clientXID=(gulong)xfdashboard_window_tracker_window_x11_get_xid(window);
		}
		if(clientXID==0) clientXID=(gulong)xWindowID;

		frozenCacheXID=(Window)clientXID;
		if(inoutFrozenCacheXID) *inoutFrozenCacheXID=frozenCacheXID;
	}

	textureComponents=cogl_texture_get_components(texture);
	hasAlpha=(textureComponents==COGL_TEXTURE_COMPONENTS_RGBA);
	textureFormat=(hasAlpha ? COGL_PIXEL_FORMAT_RGBA_8888_PRE : COGL_PIXEL_FORMAT_RGB_888);
	textureWidth=cogl_texture_get_width(texture);
	textureHeight=cogl_texture_get_height(texture);
	if(textureWidth==0 || textureHeight==0) return;

	textureSize=cogl_texture_get_data(texture, textureFormat, 0, NULL);
	if(textureSize<=0) return;

	textureData=g_try_malloc(textureSize);
	if(!textureData) return;

	gotSize=cogl_texture_get_data(texture, textureFormat, 0, textureData);
	if(gotSize<=0)
	{
		g_free(textureData);
		return;
	}

	if(!hasAlpha)
	{
		if(textureSize<(gint)(textureWidth*textureHeight*3))
		{
			g_free(textureData);
			return;
		}

		pixbuf=gdk_pixbuf_new_from_data(textureData,
										GDK_COLORSPACE_RGB,
										FALSE,
										8,
										textureWidth,
										textureHeight,
										textureWidth*3,
										xfdashboard_window_content_frozen_pixbuf_free_data,
										NULL);
		if(!pixbuf)
		{
			g_free(textureData);
			return;
		}

		xfdashboard_thumbnail_cache_save_pixbuf(pixbuf, (gulong)frozenCacheXID);
		g_object_unref(pixbuf);
	}
		else
		{
			guint8							*rgbData;
			guint							x, y;

			if(textureSize<(gint)(textureWidth*textureHeight*4))
			{
				g_free(textureData);
				return;
			}

			/* JPEG has no alpha: strip (and unpremultiply) to RGB */
			rgbData=g_try_malloc(textureWidth*textureHeight*3);
			if(!rgbData)
			{
				g_free(textureData);
				return;
			}

			for(y=0; y<textureHeight; y++)
			{
				for(x=0; x<textureWidth; x++)
				{
					guint8				r, g, b, a;

					r=textureData[(y*textureWidth+x)*4+0];
					g=textureData[(y*textureWidth+x)*4+1];
					b=textureData[(y*textureWidth+x)*4+2];
					a=textureData[(y*textureWidth+x)*4+3];
					if(a!=0 && a!=255)
					{
						r=(guint8)MIN(255, (r*255)/a);
						g=(guint8)MIN(255, (g*255)/a);
						b=(guint8)MIN(255, (b*255)/a);
					}
					rgbData[(y*textureWidth+x)*3+0]=r;
					rgbData[(y*textureWidth+x)*3+1]=g;
					rgbData[(y*textureWidth+x)*3+2]=b;
				}
			}
			g_free(textureData);

			pixbuf=gdk_pixbuf_new_from_data(rgbData,
											GDK_COLORSPACE_RGB,
											FALSE,
											8,
											textureWidth,
											textureHeight,
											textureWidth*3,
											xfdashboard_window_content_frozen_pixbuf_free_data,
											NULL);
			if(!pixbuf)
			{
				g_free(rgbData);
				return;
			}

			xfdashboard_thumbnail_cache_save_pixbuf(pixbuf, (gulong)frozenCacheXID);
			g_object_unref(pixbuf);
		}
}

/* Throttle for damage-triggered frozen saves (monotonic usec, 5s).
 * Returns TRUE once when a save should happen and updates *lastUsec.
 */
gboolean xfdashboard_window_content_frozen_should_save_damage(gint64 *lastUsec)
{
	gint64								now;

	g_return_val_if_fail(lastUsec!=NULL, FALSE);

	now=g_get_monotonic_time();
	if(*lastUsec==0 ||
		(now-*lastUsec)>(5*G_USEC_PER_SEC))
	{
		*lastUsec=now;
		return(TRUE);
	}

	return(FALSE);
}
