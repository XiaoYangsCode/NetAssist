#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class LineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    LineEditDelegate(QObject *parent = nullptr);

public:
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

signals:

};

#endif // LINEEDITDELEGATE_H
