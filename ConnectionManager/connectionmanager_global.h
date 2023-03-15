#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CONNECTIONMANAGER_LIB)
#  define CONNECTIONMANAGER_EXPORT Q_DECL_EXPORT
# else
#  define CONNECTIONMANAGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define CONNECTIONMANAGER_EXPORT
#endif
