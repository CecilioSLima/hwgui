/*
 * $Id: window.c,v 1.19 2005-11-03 19:47:37 alkresin Exp $
 *
 * HWGUI - Harbour Linux (GTK) GUI library source code:
 * C level windows functions
 *
 * Copyright 2004 Alexander S.Kresin <alex@belacy.belgorod.su>
 * www - http://kresin.belgorod.su
*/

#include "guilib.h"
#include "hbapifs.h"
#include "hbapiitm.h"
#include "hbvm.h"
#include "hbstack.h"
#include "item.api"
#include "gtk/gtk.h"
#ifdef __XHARBOUR__
#include "hbfast.h"
#endif

#define WM_MOVE                           3
#define WM_SIZE                           5
#define WM_KEYDOWN                      256    // 0x0100
#define WM_KEYUP                        257    // 0x0101
#define WM_MOUSEMOVE                    512    // 0x0200
#define WM_LBUTTONDOWN                  513    // 0x0201
#define WM_LBUTTONUP                    514    // 0x0202
#define WM_LBUTTONDBLCLK                515    // 0x0203
#define WM_RBUTTONDOWN                  516    // 0x0204
#define WM_RBUTTONUP                    517    // 0x0205


extern void writelog( char*s );

void SetObjectVar( PHB_ITEM pObject, char* varname, PHB_ITEM pValue );
PHB_ITEM GetObjectVar( PHB_ITEM pObject, char* varname );
void SetWindowObject( GtkWidget * hWnd, PHB_ITEM pObject );
void all_signal_connect( gpointer hWnd );
void cb_signal( GtkWidget *widget,gchar* data );
gint cb_signal_size( GtkWidget *widget, GtkAllocation *allocation, gpointer data );
void set_event( gpointer handle, char * cSignal, long int p1, long int p2, long int p3 );

PHB_DYNS pSym_onEvent = NULL;

typedef struct
{
   char * cName;
   int msg;
} HW_SIGNAL, * PHW_SIGNAL;

#define NUMBER_OF_SIGNALS   1
static HW_SIGNAL aSignals[NUMBER_OF_SIGNALS] = { { "destroy",2 } };

#ifndef __XHARBOUR__
#ifdef __EXPORT__
PHB_ITEM hb_stackReturn( void )
{
   HB_STACK stack = hb_GetStack();
   return &stack.Return;
}
#endif
#endif

HB_FUNC( HWG_GTK_INIT )
{
   gtk_set_locale();
   gtk_init( 0,0 );
}

HB_FUNC( HWG_GTK_EXIT )
{
   gtk_main_quit();
}

/*  Creates main application window
    InitMainWindow( szAppName, cTitle, cMenu, hIcon, nBkColor, nStyle, nLeft, nTop, nWidth, nHeight )
*/
HB_FUNC( HWG_INITMAINWINDOW )
{
   GtkWidget * hWnd ;
   GtkWidget *vbox;   
   GtkFixed * box;
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );
   // char *szAppName = hb_parc(2);
   char *cTitle = hb_parc( 3 );
   // LONG nStyle =  hb_parnl(7);
   // char *cMenu = hb_parc( 4 );
   int x = hb_parnl(8);
   int y = hb_parnl(9);
   int width = hb_parnl(10);
   int height = hb_parnl(11);

   PHB_ITEM temp;

   hWnd = ( GtkWidget * ) gtk_window_new( GTK_WINDOW_TOPLEVEL );

   cTitle = g_locale_to_utf8( cTitle,-1,NULL,NULL,NULL );
   gtk_window_set_title( GTK_WINDOW(hWnd), cTitle );
   g_free( cTitle );
   gtk_window_set_policy( GTK_WINDOW(hWnd), TRUE, TRUE, FALSE );
   gtk_window_set_default_size( GTK_WINDOW(hWnd), width, height );
   gtk_window_move( GTK_WINDOW(hWnd), x, y );

   vbox = gtk_vbox_new (FALSE, 0);
   gtk_container_add (GTK_CONTAINER(hWnd), vbox);
   // gtk_widget_show (vbox);

   box = (GtkFixed*)gtk_fixed_new();
   // gtk_container_add( GTK_CONTAINER(hWnd), (GtkWidget*)box );
   gtk_box_pack_end( GTK_BOX(vbox), (GtkWidget*)box, TRUE, TRUE, 0 );

   temp = HB_PUTHANDLE( NULL, box );
   SetObjectVar( pObject, "_FBOX", temp );
   hb_itemRelease( temp );

   temp = hb_itemPutNL( NULL, 1 );
   SetObjectVar( pObject, "_NHOLDER", temp );
   hb_itemRelease( temp );
   
   SetWindowObject( hWnd, pObject );
   g_object_set_data( (GObject*) hWnd, "fbox", (gpointer) box );
   all_signal_connect( G_OBJECT (hWnd) );
   g_signal_connect_after( box, "size-allocate", G_CALLBACK (cb_signal_size), NULL );
   set_event( (gpointer)hWnd, "configure_event", 0, 0, 0 );

   HB_RETHANDLE( hWnd );
}

HB_FUNC( HWG_CREATEDLG )
{
   GtkWidget * hWnd;
   GtkWidget * vbox;
   GtkFixed  * box;
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );
   char *cTitle = hb_itemGetCPtr( GetObjectVar( pObject, "TITLE" ) );
   int x = hb_itemGetNI( GetObjectVar( pObject, "NLEFT" ) );
   int y = hb_itemGetNI( GetObjectVar( pObject, "NTOP" ) );
   int width = hb_itemGetNI( GetObjectVar( pObject, "NWIDTH" ) );
   int height = hb_itemGetNI( GetObjectVar( pObject, "NHEIGHT" ) );
   PHB_ITEM temp;


   hWnd = ( GtkWidget * ) gtk_window_new( GTK_WINDOW_TOPLEVEL );

   cTitle = g_locale_to_utf8( cTitle,-1,NULL,NULL,NULL );
   gtk_window_set_title( GTK_WINDOW(hWnd), cTitle );
   g_free( cTitle );
   gtk_window_set_policy( GTK_WINDOW(hWnd), TRUE, TRUE, FALSE );
   gtk_window_set_default_size( GTK_WINDOW(hWnd), width, height );
   gtk_window_move( GTK_WINDOW(hWnd), x, y );

   vbox = gtk_vbox_new (FALSE, 0);
   gtk_container_add (GTK_CONTAINER(hWnd), vbox);

   box = (GtkFixed*)gtk_fixed_new();
   gtk_box_pack_end( GTK_BOX(vbox), (GtkWidget*)box, TRUE, TRUE, 0 );
   temp = HB_PUTHANDLE( NULL, box );
   SetObjectVar( pObject, "_FBOX", temp );
   hb_itemRelease( temp );

   temp = hb_itemPutNL( NULL, 1 );
   SetObjectVar( pObject, "_NHOLDER", temp );
   hb_itemRelease( temp );
   
   SetWindowObject( hWnd, pObject );
   g_object_set_data( (GObject*) hWnd, "fbox", (gpointer) box );
   all_signal_connect( G_OBJECT (hWnd) );
   g_signal_connect( box, "size-allocate", G_CALLBACK (cb_signal_size), NULL );
   set_event( (gpointer)hWnd, "configure_event", 0, 0, 0 );

   HB_RETHANDLE( hWnd );

}

/*
 *  HWG_ACTIVATEMAINWINDOW( lShow, hAccel, lMaximize, lMinimize )
 */
HB_FUNC( HWG_ACTIVATEMAINWINDOW )
{
   GtkWidget * hWnd = (GtkWidget*) HB_PARHANDLE(1);
   // HACCEL hAcceler = ( ISNIL(2) )? NULL : (HACCEL) hb_parnl(2);

   if( !ISNIL(3) && hb_parl(3) )
   {
      gtk_window_maximize( (GtkWindow*) hWnd );
   }
   if( !ISNIL(4) && hb_parl(4) )
   {
      gtk_window_iconify( (GtkWindow*) hWnd );
   }

   gtk_widget_show_all( hWnd );
   gtk_main();
}

HB_FUNC( HWG_ACTIVATEDIALOG )
{
   gtk_widget_show_all( (GtkWidget*) HB_PARHANDLE(1) );
   // gtk_dialog_run( (GtkDialog*) hb_parnl(1) );
   if( ISNIL(2) || !hb_parl(2) )
      gtk_main();
}

void ProcessMessage( void )
{
   while( g_main_context_iteration( NULL, FALSE ) );
}

HB_FUNC( HWG_PROCESSMESSAGE )
{
   ProcessMessage();
}

gint cb_signal_size( GtkWidget *widget, GtkAllocation *allocation, gpointer data )
{
   gpointer gObject = g_object_get_data( (GObject*) widget->parent->parent, "obj" );
   HB_SYMBOL_UNUSED( data );

   if( !pSym_onEvent )
      pSym_onEvent = hb_dynsymFindName( "ONEVENT" );

   if( pSym_onEvent && gObject )
   {
      PHB_ITEM pObject = hb_itemNew( NULL );
      LONG p3 = ( (ULONG)(allocation->width) & 0xFFFF ) |
                 ( ( (ULONG)(allocation->height) << 16 ) & 0xFFFF0000 );

      pObject->type = HB_IT_OBJECT;
      pObject->item.asArray.value = (PHB_BASEARRAY) gObject;
      #ifndef UIHOLDERS
      pObject->item.asArray.value->ulHolders++;
      #else
      pObject->item.asArray.value->uiHolders++;
      #endif

      /* g_signal_handlers_block_matched( (gpointer)widget, G_SIGNAL_MATCH_FUNC,
          0, 0, 0, G_CALLBACK (cb_signal_size), 0 ); */

      hb_vmPushSymbol( pSym_onEvent->pSymbol );
      hb_vmPush( pObject );
      hb_vmPushLong( WM_SIZE );
      hb_vmPushLong( 0 );
      hb_vmPushLong( p3 );
      hb_vmSend( 3 );
      hb_itemRelease( pObject );
      
      /* g_signal_handlers_unblock_matched( (gpointer)widget, G_SIGNAL_MATCH_FUNC,
          0, 0, 0, G_CALLBACK (cb_signal_size), 0 ); */   
   }
   return 0;
}

void cb_signal( GtkWidget *widget,gchar* data )
{
   gpointer gObject;
   LONG p1, p2, p3;

   // writelog( "cb_signal-0" );
   // writelog( (char*)data );
   sscanf( (char*)data,"%ld %ld %ld",&p1,&p2,&p3 );
   if( !p1 )
   {
      p1 = 273;
      widget = (GtkWidget*) p3;
      p3 = 0;
   }

   gObject = g_object_get_data( (GObject*) widget, "obj" );

   if( !pSym_onEvent )
      pSym_onEvent = hb_dynsymFindName( "ONEVENT" );

   if( pSym_onEvent && gObject )
   {
      PHB_ITEM pObject = hb_itemNew( NULL );

      pObject->type = HB_IT_OBJECT;
      pObject->item.asArray.value = (PHB_BASEARRAY) gObject;
      #ifndef UIHOLDERS
      pObject->item.asArray.value->ulHolders++;
      #else
      pObject->item.asArray.value->uiHolders++;
      #endif

      hb_vmPushSymbol( pSym_onEvent->pSymbol );
      hb_vmPush( pObject );
      hb_vmPushLong( p1 );
      hb_vmPushLong( p2 );
      hb_vmPushLong( (LONG) p3 );
      hb_vmSend( 3 );
      // res = hb_itemGetNL( (PHB_ITEM) hb_stackReturn() );
      hb_itemRelease( pObject );
   }
}

static gint cb_event( GtkWidget *widget, GdkEvent * event, gchar* data )
{
   gpointer gObject = g_object_get_data( (GObject*) widget, "obj" );
   LONG lRes;

   if( !pSym_onEvent )
      pSym_onEvent = hb_dynsymFindName( "ONEVENT" );

   //if( !gObject )
   //   gObject = g_object_get_data( (GObject*) (widget->parent->parent), "obj" );
   if( pSym_onEvent && gObject )
   {
      PHB_ITEM pObject;
      LONG p1, p2, p3;

      if( event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE )
      {
         p1 = (event->type==GDK_KEY_PRESS)? WM_KEYDOWN : WM_KEYUP;
	 p2 = ((GdkEventKey*)event)->keyval;
	 p3 = ( ( ((GdkEventKey*)event)->state & GDK_SHIFT_MASK )? 1 : 0 ) |
	      ( ( ((GdkEventKey*)event)->state & GDK_CONTROL_MASK )? 2 : 0 ) |
	      ( ( ((GdkEventKey*)event)->state & GDK_MOD1_MASK )? 4 : 0 );
      }
      else if( event->type == GDK_BUTTON_PRESS || 
               event->type == GDK_2BUTTON_PRESS ||
	       event->type == GDK_BUTTON_RELEASE )
      {
         p1 = (event->type==GDK_BUTTON_PRESS)? WM_LBUTTONDOWN : 
	      ( (event->type==GDK_BUTTON_RELEASE)? WM_LBUTTONUP : WM_LBUTTONDBLCLK );
	 p2 = 0;
	 p3 = ( ((ULONG)(((GdkEventButton*)event)->x)) & 0xFFFF ) | ( ( ((ULONG)(((GdkEventButton*)event)->y)) << 16 ) & 0xFFFF0000 );
      }
      else if( event->type == GDK_MOTION_NOTIFY )
      {
         p1 = WM_MOUSEMOVE;
	 p2 = ( ((GdkEventKey*)event)->state & GDK_BUTTON1_MASK )? 1:0;
	 p3 = ( ((ULONG)(((GdkEventMotion*)event)->x)) & 0xFFFF ) | ( ( ((ULONG)(((GdkEventMotion*)event)->y)) << 16 ) & 0xFFFF0000 );
      }
      else if( event->type == GDK_CONFIGURE )
      {
         p2 = 0;
         if( widget->allocation.width != ((GdkEventConfigure*)event)->width ||
             widget->allocation.height!= ((GdkEventConfigure*)event)->height )
         {
            return 0;
         }
         else
         {
            p1 = WM_MOVE;
            p3 = ( ((GdkEventConfigure*)event)->x & 0xFFFF ) |
                 ( ( ((GdkEventConfigure*)event)->y << 16 ) & 0xFFFF0000 );
         }
      }
      else if( event->type == GDK_ENTER_NOTIFY || event->type == GDK_LEAVE_NOTIFY )
      {
         p1 = WM_MOUSEMOVE;
         p2 = ( ((GdkEventCrossing*)event)->state & GDK_BUTTON1_MASK )? 1:0 | 
              ( event->type == GDK_ENTER_NOTIFY )? 0x10:0;
         p3 = ( ((ULONG)(((GdkEventCrossing*)event)->x)) & 0xFFFF ) | ( ( ((ULONG)(((GdkEventMotion*)event)->y)) << 16 ) & 0xFFFF0000 );
      }
      else
         sscanf( (char*)data,"%ld %ld %ld",&p1,&p2,&p3 );

      pObject = hb_itemNew( NULL );
      pObject->type = HB_IT_OBJECT;
      pObject->item.asArray.value = (PHB_BASEARRAY) gObject;
      #ifndef UIHOLDERS
      pObject->item.asArray.value->ulHolders++;
      #else
      pObject->item.asArray.value->uiHolders++;
      #endif

      hb_vmPushSymbol( pSym_onEvent->pSymbol );
      hb_vmPush( pObject );
      hb_vmPushLong( p1 );
      hb_vmPushLong( p2 );
      hb_vmPushLong( p3 );
      hb_vmSend( 3 );
      lRes = hb_parnl( -1 );
      hb_itemRelease( pObject );
      return lRes;
   }
   return 0;
}

void set_signal( gpointer handle, char * cSignal, long int p1, long int p2, long int p3 )
{
   char buf[25]={0};

   sprintf( buf, "%ld %ld %ld", p1, p2, p3 );
   g_signal_connect( handle, cSignal,
                      G_CALLBACK (cb_signal), g_strdup(buf) );
}

HB_FUNC( HWG_SETSIGNAL )
{
   gpointer p = (gpointer) HB_PARHANDLE(1);
   set_signal( (gpointer)p, hb_parc(2), hb_parnl(3), hb_parnl(4), ( long int ) HB_PARHANDLE( 5 ) );
}

void set_event( gpointer handle, char * cSignal, long int p1, long int p2, long int p3 )
{
   char buf[25]={0};

   sprintf( buf, "%ld %ld %ld", p1, p2, p3 );
   g_signal_connect( handle, cSignal,
                      G_CALLBACK (cb_event), g_strdup(buf) );
}

HB_FUNC( HWG_SETEVENT )
{
   gpointer p = (gpointer) HB_PARHANDLE(1);
   set_event( p, hb_parc(2), hb_parnl(3), hb_parnl(4), hb_parnl(5) );
}

void all_signal_connect( gpointer hWnd )
{
   int i;
   char buf[20]={0};

   // writelog( "all_signal-connect-0" );
   for( i=0; i<NUMBER_OF_SIGNALS; i++ )
   {
      sprintf( buf,"%d 0 0",aSignals[i].msg );
      // writelog(buf);
      g_signal_connect( hWnd, aSignals[i].cName,
        G_CALLBACK (cb_signal), g_strdup(buf) );
   }
}

GtkWidget * GetActiveWindow( void )
{
   return gtk_window_list_toplevels()->data;
}

HB_FUNC( GETACTIVEWINDOW )
{
   HB_RETHANDLE( GetActiveWindow() );
}

HB_FUNC( SETWINDOWOBJECT )
{
   SetWindowObject( (GtkWidget *) HB_PARHANDLE(1),hb_param(2,HB_IT_OBJECT) );
}

void SetWindowObject( GtkWidget * hWnd, PHB_ITEM pObject )
{
   if( pObject )
   {
      // Must increase uiHolders as we now have additional copy of object.
      #ifndef UIHOLDERS
      pObject->item.asArray.value->ulHolders++;
      #else
      pObject->item.asArray.value->uiHolders++;
      #endif
      g_object_set_data( (GObject*) hWnd, "obj", (gpointer) (pObject->item.asArray.value) );
   }
   else
   {
      g_object_set_data( (GObject*) hWnd, "obj", (gpointer) NULL );
   }
}

HB_FUNC( GETWINDOWOBJECT )
{
   gpointer dwNewLong = g_object_get_data( (GObject*) HB_PARHANDLE(1), "obj" );

   if( dwNewLong )
   {
      PHB_ITEM pObj = hb_itemNew( NULL );

      pObj->type = HB_IT_OBJECT;
      pObj->item.asArray.value = (PHB_BASEARRAY) dwNewLong;

      // Must increase uiHolders as we will shortly release this unaccounted copy.
      #ifndef UIHOLDERS
      pObj->item.asArray.value->ulHolders++;
      #else
      pObj->item.asArray.value->uiHolders++;
      #endif

      hb_itemReturn( pObj );
      hb_itemRelease( pObj );
   }
}

HB_FUNC( SETWINDOWTEXT )
{
   char * cTitle = g_locale_to_utf8( hb_parc(2),-1,NULL,NULL,NULL );
   gtk_window_set_title( GTK_WINDOW( HB_PARHANDLE(1) ), cTitle );
   g_free( cTitle );
}

HB_FUNC( GETWINDOWTEXT )
{
   char * cTitle = (char*) gtk_window_get_title( GTK_WINDOW( HB_PARHANDLE(1) ) );

   if( cTitle )
      hb_retc( cTitle );
   else
      hb_retc( "" );
}

HB_FUNC( ENABLEWINDOW )
{
   GtkWidget * widget = (GtkWidget*) HB_PARHANDLE( 1 );
   BOOL lEnable = hb_parl( 2 );
   gtk_widget_set_sensitive( widget, lEnable );
}

HB_FUNC( ISWINDOWENABLED )
{
   hb_retl( GTK_WIDGET_IS_SENSITIVE( (GtkWidget*) HB_PARHANDLE(1) ) );
}

HB_FUNC( MOVEWINDOW )
{
   GtkWidget * hWnd = (GtkWidget*)HB_PARHANDLE(1);

   if( !ISNIL(2) || !ISNIL(3) )
      gtk_window_move( GTK_WINDOW(hWnd), hb_parni(2), hb_parni(3) );
   if( !ISNIL(4) || !ISNIL(5) )
      gtk_window_resize( GTK_WINDOW(hWnd), hb_parni(4), hb_parni(5) );
}

HB_FUNC( HWG_WINDOWMAXIMIZE )
{

   gtk_window_maximize( (GtkWindow*) HB_PARHANDLE(1) );
}

HB_FUNC( HWG_WINDOWRESTORE )
{

   gtk_window_unmaximize( (GtkWindow*) HB_PARHANDLE(1) );
}

HB_FUNC( HWG_WINDOWMINIMIZE )
{

   gtk_window_iconify( (GtkWindow*) HB_PARHANDLE(1) );
}

PHB_ITEM GetObjectVar( PHB_ITEM pObject, char* varname )
{
#ifdef __XHARBOUR__
   return hb_objSendMsg( pObject, varname, 0 );
#else
   hb_objSendMsg( pObject, varname, 0 );
#ifndef HARBOUR_OLD_VERSION
   return ( hb_stackReturnItem() );
#else
   return ( hb_stackReturn() );
#endif
#endif
}
            
void SetObjectVar( PHB_ITEM pObject, char* varname, PHB_ITEM pValue )
{
   hb_objSendMsg( pObject, varname, 1, pValue );
}

/*               
PHB_ITEM GetObjectVar( PHB_ITEM pObject, char* varname )
{
   PHB_DYNS pMsg = hb_dynsymGet( varname );

   if( pMsg )
   {
      hb_vmPushSymbol( pMsg->pSymbol );
      hb_vmPush( pObject );

      hb_vmDo( 0 );
   }
   #ifndef __XHARBOUR__
   return ( hb_stackReturn() );
   #else
   return ( hb_stackReturnItem() );
   #endif
}

void SetObjectVar( PHB_ITEM pObject, char* varname, PHB_ITEM pValue )
{
   PHB_DYNS pMsg = hb_dynsymGet( varname );

   if( pMsg )
   {
      hb_vmPushSymbol( pMsg->pSymbol );
      hb_vmPush( pObject );
      hb_vmPush( pValue );

      hb_vmDo( 1 );
   }
}
*/

HB_FUNC( HWG_DECREASEHOLDERS )
{
   PHB_ITEM pObject = hb_param( 1, HB_IT_OBJECT );

   #ifndef  UIHOLDERS
   if( pObject->item.asArray.value->ulHolders )
      pObject->item.asArray.value->ulHolders--;
   #else
   if( pObject->item.asArray.value->uiHolders )
      pObject->item.asArray.value->uiHolders--;
   #endif
}

HB_FUNC( SETFOCUS )
{
   gtk_widget_grab_focus( (GtkWidget*) HB_PARHANDLE( 1 ) );
}

HB_FUNC( GETFOCUS )
{
   GtkWidget * hCtrl;
   hCtrl = gtk_window_get_focus( gtk_window_list_toplevels()->data );
}

HB_FUNC( HWG_DESTROYWINDOW )
{
    gtk_widget_destroy( (GtkWidget *) HB_PARHANDLE(1) );
}

HB_FUNC( HWG_SET_MODAL )
{
   gtk_window_set_modal( (GtkWindow *) HB_PARHANDLE(1), 1 );
   gtk_window_set_transient_for( (GtkWindow *) HB_PARHANDLE(1), (GtkWindow *) HB_PARHANDLE(2) );
}