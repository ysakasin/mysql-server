/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA */

#ifndef PFS_NOTIFY_SERVICE_H
#define PFS_NOTIFY_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/psi/mysql_thread.h>

/**
  @page PAGE_PFS_NOTIFICATION_SERVICE Performance Schema Notification service

  @section PFS_NOTIFICATION_INTRO Introduction
  The Performance Schema Notification service provides a means to register
  user-defined callback functions for the following thread and session events:

  - thread create
  - thread destroy
  - session connect
  - session disconnect
  - session change user

  This synchronous, low-level API is designed to impose minimal performance
  overhead.

  @section PFS_NOTIFICATION_CALLBACKS Callback Function Definition

  Callback functions are of the type #PSI_notification_cb:

  @verbatim
    typedef void (*PSI_notification_cb)(const PSI_thread_attrs *thread_attrs);
  @endverbatim

  For example:
  @verbatim
    void callback_thread_create(const PSI_thread_attrs *thread_attrs)
    {
      ...
    }
  @endverbatim

  When the callback is invoked, the thread attributes structure will contain the
  system attributes of the thread.
  
  @section PFS_NOTIFICATION_REGISTER Registering Events

  To register for one or more events, set the corresponding callback function
  pointers in the #PSI_notification structure, leaving the function pointer
  NULL for unused callbacks.
  
  Use the service function @c register_notification() to register the callbacks
  with the server

  @verbatim
    int register_notification(PSI_notification *callbacks,
                              bool with_ref_count);
  @endverbatim

  where
  @verbatim
    callbacks      Callback function set
    with_ref_count Set TRUE for callback functions in dynamically loaded
                   modules. Set FALSE for callback functions in static or
                   unloadable modules.
  @endverbatim
  
  For example:
  @verbatim
    PSI_notification_cb my_callbacks;

    my_callbacks.thread_create   = &thread_create_callback;
    my_callbacks.thread_destroy  = &thread_destroy_callback;
    my_callbacks.session_connect = &session_connect_callback;
    my_callbacks.session_disconnect  = &session_disconnect_callback;
    my_callbacks.session_change_user = nullptr;

    int my_handle =
     mysql_service_pfs_notification->register_notification(&my_callbacks, true);
  @endverbatim

  A non-zero handle is returned if the registration is successful. This handle
  is used to unregister the callback set.

  A callback set can be registered more than once. No error is returned
  for calling @c register_notification() more than once for a given callback set.
  Callbacks are invoked once for each time they are registered.

  For callback functions that reside in dynamically loaded modules, set
  @verbatim with_ref_count = TRUE @endverbatim so that the module can be safely unloaded after
  the callbacks are unregistered.

  For callback functions that reside in static, built-in or otherwise unloadable
  modules, set @verbatim with_ref_count = FALSE @endverbatim to optimize callback performance in
  high-concurrency environments.

  Callbacks that reside in a dynamically loaded module such as a server plugin,
  must be successfully unregistered before the module is unloaded.
  
  For callbacks in static or unloadable modules, @c unregister_notification() will disable the
  callback functions, but the function pointers will remain.

  @section PFS_NOTIFICATION_UNREGISTER Unregistering or Disabling Events

  To unregister callback functions from the Notification service, use the handle
  returned from @c register_notification(). For example:

  @verbatim
    int ret = mysql_service_pfs_notification->unregister_notification(my_handle);

    if (ret == 0)
    {
      // unload component
    }
    else
    {
      // error
    }

  @endverbatim

  Callbacks that reside in a dynamically loaded module such as a server plugin or
  component must be successfully unregistered before the module is unloaded.

  If @c unregister_notification() returns an error, then the module should not
  be unloaded.

  If the callbacks were registered with @verbatim with_ref_count = TRUE @endverbatim then
  @c unregister_notification() will return an error if any of the functions are in use
  and fail to return after 2 seconds.

  If the callbacks were registered with @verbatim with_ref_count = FALSE @endverbatim then
  @c unregister_notification() will disable the callback functions, but the
  callback function pointers will be assumed to be valid until the server is shutdown.

  @c unregister_callback() can be called multiple times for the same handle.

  Failing to unregister all callback functions from a dynamically loaded plugin
  or component may result in a undocumented behavior during server shutdown.

*/

BEGIN_SERVICE_DEFINITION(pfs_notification)
  register_notification_v1_t register_notification;
  unregister_notification_v1_t unregister_notification;
END_SERVICE_DEFINITION(pfs_notification)

#endif
