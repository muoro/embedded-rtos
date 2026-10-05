#pragma once
#include <QAbstractListModel>
#include <QVector>

// Bounded presentation log. Rows are structured so QML never parses log strings.
class EventListModel final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
  public:
    enum Role { TimeRole = Qt::UserRole + 1, CategoryRole, DescriptionRole };
    explicit EventListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}
    int rowCount(const QModelIndex& parent = {}) const override;
    int count() const { return rowCount(); }
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void append(const QString& category, const QString& description);
  signals:
    void countChanged();
  private:
    struct Entry { QString time, category, description; };
    static constexpr int maximumEntries = 60;
    QVector<Entry> entries_;
};
