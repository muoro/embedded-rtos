#include "EventListModel.hpp"
#include <QDateTime>

int EventListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(entries_.size());
}

QVariant EventListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};
    const auto& entry = entries_.at(index.row());
    switch (role) {
    case TimeRole: return entry.time;
    case CategoryRole: return entry.category;
    case DescriptionRole: return entry.description;
    default: return {};
    }
}

QHash<int, QByteArray> EventListModel::roleNames() const {
    return {{TimeRole, "timestamp"}, {CategoryRole, "category"}, {DescriptionRole, "description"}};
}

void EventListModel::append(const QString& category, const QString& description) {
    if (entries_.size() == maximumEntries) {
        beginRemoveRows({}, maximumEntries - 1, maximumEntries - 1);
        entries_.removeLast();
        endRemoveRows();
    }
    beginInsertRows({}, 0, 0);
    entries_.prepend({QDateTime::currentDateTime().toString("HH:mm:ss"), category, description});
    endInsertRows();
    emit countChanged();
}
