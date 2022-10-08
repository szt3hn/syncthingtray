#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "./settings.h"

#include "../webview/webviewdefs.h"

#include <syncthingmodel/syncthingicons.h>

#include <qtutilities/settingsdialog/optionpage.h>
#include <qtutilities/settingsdialog/qtsettings.h>
#include <qtutilities/settingsdialog/settingsdialog.h>

#include <QProcess>
#include <QStringList>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QLabel)

namespace CppUtilities {
class DateTime;
}

namespace QtUtilities {
class ColorButton;
class IconButton;
} // namespace QtUtilities

namespace Data {
class SyncthingConnection;
class SyncthingService;
class SyncthingProcess;
class SyncthingLauncher;
class SyncthingStatusComputionModel;
} // namespace Data

namespace QtGui {

/*!
 * \brief The GuiType enum specifies a GUI type.
 *
 * Such a value can be passed to some option pages to show only the options which are relevant
 * for the particular GUI type.
 */
enum class GuiType {
    TrayWidget,
    Plasmoid,
};

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE_CUSTOM_CTOR(ConnectionOptionPage)
public:
ConnectionOptionPage(Data::SyncthingConnection *connection, QWidget *parentWidget = nullptr);
void hideConnectionStatus();

private:
DECLARE_SETUP_WIDGETS
void insertFromConfigFile(bool forceFileSelection);
void updateConnectionStatus();
void applyAndReconnect();
bool showConnectionSettings(int index);
bool cacheCurrentSettings(bool applying);
void saveCurrentConfigName(const QString &name);
void addNewConfig();
void removeSelectedConfig();
void moveSelectedConfigDown();
void moveSelectedConfigUp();
void setCurrentIndex(int currentIndex);
Data::SyncthingConnection *m_connection;
Data::SyncthingConnectionSettings m_primarySettings;
std::vector<Data::SyncthingConnectionSettings> m_secondarySettings;
Data::SyncthingStatusComputionModel *m_statusComputionModel;
int m_currentIndex;
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE_CUSTOM_CTOR(NotificationsOptionPage)
public:
NotificationsOptionPage(GuiType guiType = GuiType::TrayWidget, QWidget *parentWidget = nullptr);

private:
DECLARE_SETUP_WIDGETS
const GuiType m_guiType;
END_DECLARE_OPTION_PAGE

DECLARE_UI_FILE_BASED_OPTION_PAGE(AppearanceOptionPage)

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE_CUSTOM_CTOR(IconsOptionPage)
public:
enum class Context { Combined, UI, System };
explicit IconsOptionPage(Context context = Context::Combined, QWidget *parentWidget = nullptr);
DECLARE_SETUP_WIDGETS
private:
void update();
Context m_context;
Data::StatusIconSettings m_settings;
struct {
    QtUtilities::ColorButton *colorButtons[3] = {};
    QLabel *previewLabel = nullptr;
    Data::StatusIconColorSet *setting = nullptr;
    Data::StatusEmblem statusEmblem = Data::StatusEmblem::None;
} m_widgets[Data::StatusIconSettings::distinguishableColorCount];
END_DECLARE_OPTION_PAGE

DECLARE_UI_FILE_BASED_OPTION_PAGE_CUSTOM_SETUP(AutostartOptionPage)
SYNCTHINGWIDGETS_EXPORT bool isAutostartEnabled();
SYNCTHINGWIDGETS_EXPORT bool setAutostartEnabled(bool enabled);

BEGIN_DECLARE_TYPEDEF_UI_FILE_BASED_OPTION_PAGE(LauncherOptionPage)
class QT_UTILITIES_EXPORT LauncherOptionPage : public QObject, public ::QtUtilities::UiFileBasedOptionPage<Ui::LauncherOptionPage> {
    Q_OBJECT

public:
    LauncherOptionPage(QWidget *parentWidget = nullptr);
    LauncherOptionPage(
        const QString &tool, const QString &toolName = QString(), const QString &windowTitle = QString(), QWidget *parentWidget = nullptr);
    ~LauncherOptionPage() override;
    bool apply() override;
    void reset() override;

private Q_SLOTS:
    void handleSyncthingLaunched(bool running);
    void handleSyncthingReadyRead();
    void handleSyncthingOutputAvailable(const QByteArray &output);
    void handleSyncthingExited(int exitCode, QProcess::ExitStatus exitStatus);
    void handleSyncthingError(QProcess::ProcessError error);
    bool isRunning() const;
    void launch();
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    void updateLibSyncthingLogLevel();
#endif
    void stop();
    void restoreDefaultArguments();

private:
    DECLARE_SETUP_WIDGETS

    Data::SyncthingProcess *const m_process;
    Data::SyncthingLauncher *const m_launcher;
    QAction *m_restoreArgsAction;
    QAction *m_syncthingDownloadAction;
    bool m_kill;
    QString m_tool, m_toolName, m_windowTitle;
};

#ifdef LIB_SYNCTHING_CONNECTOR_SUPPORT_SYSTEMD
BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE(SystemdOptionPage)
private:
DECLARE_SETUP_WIDGETS
void handleSystemUnitChanged();
void handleDescriptionChanged(const QString &description);
void handleStatusChanged(const QString &activeState, const QString &subState, CppUtilities::DateTime activeSince);
void handleEnabledChanged(const QString &unitFileState);
bool updateRunningColor();
bool updateEnabledColor();
void updateColors();
Data::SyncthingService *const m_service;
QStringList m_status;
QMetaObject::Connection m_unitChangedConn;
QMetaObject::Connection m_descChangedConn;
QMetaObject::Connection m_statusChangedConn;
QMetaObject::Connection m_enabledChangedConn;
END_DECLARE_OPTION_PAGE
#endif

#ifndef SYNCTHINGWIDGETS_NO_WEBVIEW
DECLARE_UI_FILE_BASED_OPTION_PAGE(WebViewOptionPage)
#else
DECLARE_OPTION_PAGE(WebViewOptionPage)
#endif

class SYNCTHINGWIDGETS_EXPORT SettingsDialog : public QtUtilities::SettingsDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(Data::SyncthingConnection *connection, QWidget *parent = nullptr);
    explicit SettingsDialog(const QList<QtUtilities::OptionCategory *> &categories, QWidget *parent = nullptr);
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;
    void hideConnectionStatus();
    void selectLauncherSettings();

Q_SIGNALS:
    void wizardRequested();

private:
    void init();

    ConnectionOptionPage *m_connectionsOptionPage;
    int m_launcherSettingsCategory = -1, m_launcherSettingsPageIndex = -1;
};
} // namespace QtGui

DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, ConnectionOptionPage)
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, NotificationsOptionPage)
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, AppearanceOptionPage)
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, IconsOptionPage)
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, AutostartOptionPage)
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, LauncherOptionPage)
#ifdef LIB_SYNCTHING_CONNECTOR_SUPPORT_SYSTEMD
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, SystemdOptionPage)
#endif
#ifndef SYNCTHINGWIDGETS_NO_WEBVIEW
DECLARE_EXTERN_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, WebViewOptionPage)
#endif

#endif // SETTINGS_DIALOG_H
