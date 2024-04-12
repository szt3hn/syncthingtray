#include "./syncthingfilemodel.h"
#include "./syncthingicons.h"

#include <syncthingconnector/syncthingconnection.h>
#include <syncthingconnector/utils.h>

#include <c++utilities/conversion/stringconversion.h>

#include <QStringBuilder>

#include <iostream>

using namespace std;
using namespace CppUtilities;

namespace Data {

SyncthingFileModel::SyncthingFileModel(SyncthingConnection &connection, const SyncthingDir &dir, QObject *parent)
    : SyncthingModel(connection, parent)
    , m_connection(connection)
    , m_dirId(dir.id)
    , m_root(std::make_unique<SyncthingItem>())
{
    m_root->name = dir.displayName();
    m_root->modificationTime = dir.lastFileTime;
    m_root->size = dir.globalStats.bytes;
    m_root->type = SyncthingItemType::Directory;
    m_connection.browse(m_dirId, QString(), 1, [this](std::vector<std::unique_ptr<SyncthingItem>> &&items, QString &&errorMessage) {
        Q_UNUSED(errorMessage)
        if (items.empty()) {
            return;
        }
        const auto last = items.size() - 1;
        beginInsertRows(index(0, 0), 0, last < std::numeric_limits<int>::max() ? static_cast<int>(last) : std::numeric_limits<int>::max());
        m_root->children = std::move(items);
        m_root->childrenPopulated = true;
        endInsertRows();
    });
}

SyncthingFileModel::~SyncthingFileModel()
{
    QObject::disconnect(m_pendingRequest);
}

QHash<int, QByteArray> SyncthingFileModel::roleNames() const
{
    const static auto roles = QHash<int, QByteArray>{
        { NameRole, "name" },
        { SizeRole, "size" },
        { ModificationTimeRole, "modificationTime" },
        { Actions, "actions" },
        { ActionNames, "actionNames" },
        { ActionIcons, "actionIcons" },
    };
    return roles;
}

QModelIndex SyncthingFileModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column > 2 || parent.column() > 0) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        return static_cast<std::size_t>(row) ? QModelIndex() : createIndex(row, column, m_root.get());
    }
    auto *const parentItem = reinterpret_cast<SyncthingItem *>(parent.internalPointer());
    if (!parentItem) {
        return QModelIndex();
    }
    auto &items = parentItem->children;
    if (static_cast<std::size_t>(row) >= items.size()) {
        return QModelIndex();
    }
    auto &item = items[static_cast<std::size_t>(row)];
    item->parent = parentItem;
    return createIndex(row, column, item.get());
}

QModelIndex SyncthingFileModel::index(const QString &path) const
{
    auto parts = path.split(QChar('/'), Qt::SkipEmptyParts);
    auto *parent = m_root.get();
    auto res = createIndex(0, 0, parent);
    for (const auto &part : parts) {
        auto index = 0;
        for (const auto &child : parent->children) {
            if (child->name == part) {
                child->parent = parent;
                parent = child.get();
                res = createIndex(index, 0, parent);
                index = -1;
                break;
            }
            ++index;
        }
        if (index >= 0) {
            res = QModelIndex();
            return res;
        }
    }
    std::cerr << "index for path " << path.toStdString() << ": " << this->path(res).toStdString() << '\n';
    return res;
}

QString SyncthingFileModel::path(const QModelIndex &index) const
{
    auto res = QString();
    if (!index.isValid()) {
        return res;
    }
    auto parts = QStringList();
    auto size = QString::size_type();
    parts.reserve(reinterpret_cast<SyncthingItem *>(index.internalPointer())->level + 1);
    for (auto i = index; i.isValid(); i = i.parent()) {
        const auto *const item = reinterpret_cast<SyncthingItem *>(i.internalPointer());
        if (item == m_root.get()) {
            break;
        }
        parts.append(reinterpret_cast<SyncthingItem *>(i.internalPointer())->name);
        size += parts.back().size();
    }
    res.reserve(size + parts.size());
    for (auto i = parts.rbegin(), end = parts.rend(); i != end; ++i) {
        res += *i;
        res += QChar('/');
    }
    return res;
}

QModelIndex SyncthingFileModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }
    auto *const childItem = reinterpret_cast<SyncthingItem *>(child.internalPointer());
    if (!childItem) {
        return QModelIndex();
    }
    return !childItem->parent ? QModelIndex() : createIndex(static_cast<int>(childItem->index), 0, childItem->parent);
}

QVariant SyncthingFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Size");
            case 2:
                return tr("Last modified");
            }
            break;
        default:;
        }
        break;
    default:;
    }
    return QVariant();
}

QVariant SyncthingFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    auto *const item = reinterpret_cast<SyncthingItem *>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return item->name;
        case 1:
            return QString::fromStdString(CppUtilities::dataSizeToString(item->size));
        case 2:
            return QString::fromStdString(item->modificationTime.toString());
        }
        break;
    case Qt::DecorationRole: {
        const auto &icons = commonForkAwesomeIcons();
        switch (index.column()) {
        case 0:
            switch (item->type) {
            case SyncthingItemType::File:
                return icons.file;
            case SyncthingItemType::Directory:
                return icons.folder;
            default:
                return icons.cogs;
            }
        }
        break;
    }
    case NameRole:
        return item->name;
    case SizeRole:
        return static_cast<qsizetype>(item->size);
    case ModificationTimeRole:
        return QString::fromStdString(item->modificationTime.toString());
    case Actions:
        if (item->type == SyncthingItemType::Directory) {
            return QStringList({ QStringLiteral("refresh") });
        }
        break;
    case ActionNames:
        if (item->type == SyncthingItemType::Directory) {
            return QStringList({ tr("Refresh") });
        }
        break;
    case ActionIcons:
        if (item->type == SyncthingItemType::Directory) {
            return QStringList({ QStringLiteral("view-refresh") });
        }
        break;
    }
    return QVariant();
}

bool SyncthingFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

int SyncthingFileModel::rowCount(const QModelIndex &parent) const
{
    auto res = std::size_t();
    if (!parent.isValid()) {
        res = 1;
    } else {
        auto *const parentItem = reinterpret_cast<SyncthingItem *>(parent.internalPointer());
        res = parentItem->childrenPopulated || parentItem->type != SyncthingItemType::Directory ? parentItem->children.size() : 1;
    }
    return res < std::numeric_limits<int>::max() ? static_cast<int>(res) : std::numeric_limits<int>::max();
}

int SyncthingFileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

bool SyncthingFileModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return false;
    }
    auto *const parentItem = reinterpret_cast<SyncthingItem *>(parent.internalPointer());
    return !parentItem->childrenPopulated && parentItem->type == SyncthingItemType::Directory;
}

/// \cond
static void addLevel(std::vector<std::unique_ptr<SyncthingItem>> &items, int level)
{
    for (auto &item : items) {
        item->level += level;
        addLevel(item->children, level);
    }
}

static void considerFetched(std::vector<std::unique_ptr<SyncthingItem>> &items)
{
    for (auto &item : items) {
        item->childrenPopulated = true;
        considerFetched(item->children);
    }
}
/// \endcond

void SyncthingFileModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid()) {
        return;
    }
    m_fetchQueue.append(path(parent));
    if (m_fetchQueue.size() == 1) {
        processFetchQueue();
    }
}

void SyncthingFileModel::triggerAction(const QString &action, const QModelIndex &index)
{
    if (action == QLatin1String("refresh")) {
        fetchMore(index);
    }
}

void SyncthingFileModel::handleConfigInvalidated()
{
}

void SyncthingFileModel::handleNewConfigAvailable()
{
}

void SyncthingFileModel::handleForkAwesomeIconsChanged()
{
    invalidateAllIndicies(QVector<int>({ Qt::DecorationRole }));
}

void SyncthingFileModel::processFetchQueue()
{
    if (m_fetchQueue.isEmpty()) {
        return;
    }
    const auto &path = m_fetchQueue.front();
    m_pendingRequest = m_connection.browse(
        m_dirId, path, 1, [this, p = path](std::vector<std::unique_ptr<SyncthingItem>> &&items, QString &&errorMessage) mutable {
            Q_UNUSED(errorMessage)

            {
                const auto refreshedIndex = index(p);
                if (!refreshedIndex.isValid()) {
                    m_fetchQueue.removeAll(p);
                    processFetchQueue();
                    return;
                }
                auto *const refreshedItem = reinterpret_cast<SyncthingItem *>(refreshedIndex.internalPointer());
                if (!refreshedItem->children.empty()) {
                    if (false && refreshedItem == m_root.get()) {
                        beginResetModel();
                    } else {
                        considerFetched(refreshedItem->children);
                        std::cout << "begin remove rows at: " << this->path(refreshedIndex).toStdString() << std::endl;
                        std::cout << " - from 0 to " << static_cast<int>(refreshedItem->children.size() - 1) << std::endl;
                        for (int row = 0; row < static_cast<int>(refreshedItem->children.size()); ++row) {
                            std::cout << " - " << row << " - " << index(row, 0, refreshedIndex).data().toString().toStdString() << std::endl;
                        }
                        beginRemoveRows(refreshedIndex, 0, static_cast<int>(refreshedItem->children.size() - 1));
                    }
                    std::cout << "old row count: " << rowCount(refreshedIndex) << std::endl;
                    refreshedItem->children.clear();
                    if (false && refreshedItem == m_root.get()) {
                        endResetModel();
                    } else {
                        endRemoveRows();
                    }
                    std::cout << "new row count: " << rowCount(refreshedIndex) << std::endl;
                }
            }
            if (!items.empty()) {
                QTimer::singleShot(400, this, [this, p = std::move(p), items = std::move(items)]() mutable {
                    const auto refreshedIndex = index(p);
                    if (!refreshedIndex.isValid()) {
                        m_fetchQueue.removeAll(p);
                        processFetchQueue();
                        return;
                    }
                    auto *const refreshedItem = reinterpret_cast<SyncthingItem *>(refreshedIndex.internalPointer());
                    const auto last = items.size() - 1;
                    addLevel(items, refreshedItem->level);
                    for (auto &item : items) {
                        item->parent = refreshedItem;
                    }
                    beginInsertRows(
                        refreshedIndex, 0, last < std::numeric_limits<int>::max() ? static_cast<int>(last) : std::numeric_limits<int>::max());
                    refreshedItem->children = std::move(items);
                    refreshedItem->childrenPopulated = true;
                    endInsertRows();

                    m_fetchQueue.removeAll(p);
                    processFetchQueue();
                });
            }
        });
}

} // namespace Data
