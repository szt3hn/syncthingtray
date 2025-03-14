#ifndef DATA_SYNCTHINGMODEL_H
#define DATA_SYNCTHINGMODEL_H

#include "./global.h"

#include <QAbstractItemModel>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
Q_MOC_INCLUDE("../syncthingconnector/syncthingconnection.h")
#endif

namespace Data {

class SyncthingConnection;

class LIB_SYNCTHING_MODEL_EXPORT SyncthingModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(SyncthingConnection *connection READ connection)
    Q_PROPERTY(bool brightColors READ brightColors WRITE setBrightColors)
    Q_PROPERTY(bool singleColumnMode READ singleColumnMode WRITE setSingleColumnMode)

public:
    enum SyncthingModelRole {
        IsPinned = Qt::UserRole + 1,
        SyncthingModelUserRole = Qt::UserRole + 100,
    };

    explicit SyncthingModel(SyncthingConnection &connection, QObject *parent = nullptr);
    Data::SyncthingConnection *connection();
    const Data::SyncthingConnection *connection() const;
    bool brightColors() const;
    void setBrightColors(bool brightColors);
    bool singleColumnMode() const;
    void setSingleColumnMode(bool singleColumnModeEnabled);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    virtual const QVector<int> &colorRoles() const;
    void invalidateTopLevelIndicies(const QVector<int> &affectedRoles);
    void invalidateNestedIndicies(const QVector<int> &affectedRoles);
    void invalidateAllIndicies(const QVector<int> &affectedRoles, const QModelIndex &parentIndex = QModelIndex());
    void invalidateAllIndicies(const QVector<int> &affectedRoles, int column, const QModelIndex &parentIndex = QModelIndex());

private Q_SLOTS:
    virtual void handleConfigInvalidated();
    virtual void handleNewConfigAvailable();
    virtual void handleStatusIconsChanged();
    virtual void handleForkAwesomeIconsChanged();
    virtual void handleBrightColorsChanged();

protected:
    Data::SyncthingConnection &m_connection;
    bool m_brightColors;
    bool m_singleColumnMode;
};

inline SyncthingConnection *SyncthingModel::connection()
{
    return &m_connection;
}

inline const SyncthingConnection *SyncthingModel::connection() const
{
    return &m_connection;
}

inline bool SyncthingModel::brightColors() const
{
    return m_brightColors;
}

inline bool SyncthingModel::singleColumnMode() const
{
    return m_singleColumnMode;
}

} // namespace Data

#endif // DATA_SYNCTHINGMODEL_H
