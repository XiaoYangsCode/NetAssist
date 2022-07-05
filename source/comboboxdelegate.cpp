#include "comboboxdelegate.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent/*=nullptr*/)
    : QStyledItemDelegate(parent)
{

}

void ComboBoxDelegate::setItems(const QStringList &texts)
{
    m_texts = texts;
}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(m_texts);
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void ComboBoxDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    QStyledItemDelegate::paint(painter, viewOption, index);
}



