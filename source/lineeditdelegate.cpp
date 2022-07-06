#include "lineeditdelegate.h"
#include <QLineEdit>

LineEditDelegate::LineEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget* LineEditDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    editor->setInputMask("HH HH");
    return editor;
}

void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   QString text = index.model()->data(index, Qt::EditRole).toString();
   QLineEdit *pLineEdit = static_cast<QLineEdit*>(editor);
   pLineEdit->setText(text);
}

void LineEditDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QLineEdit* pLineEdit = static_cast<QLineEdit*>(editor);
    QString text = pLineEdit->text();
    model->setData(index, text, Qt::EditRole);
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void LineEditDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    QStyledItemDelegate::paint(painter, viewOption, index);
}
