#ifndef SYNCTHINGWIDGETS_UTILS_H
#define SYNCTHINGWIDGETS_UTILS_H

#include "../global.h"

namespace Data {
class SyncthingConnection;
}

namespace QtGui {

SYNCTHINGWIDGETS_EXPORT void handleCurrentTabChanged(int tabIndex, Data::SyncthingConnection &connection);

} // namespace QtGui

#endif // SYNCTHINGWIDGETS_UTILS_H
