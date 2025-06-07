#include "controllers/inputreplacement/InputReplacementModel.hpp"

#include "controllers/inputreplacement/InputReplacement.hpp"
#include "util/StandardItemHelper.hpp"

namespace chatterino {

InputReplacementModel::InputReplacementModel(QObject *parent)
    : SignalVectorModel<InputReplacement>(5, parent)
{
}

// turn a vector item into a model row
InputReplacement InputReplacementModel::getItemFromRow(
    std::vector<QStandardItem *> &row, const InputReplacement &original)
{
    return InputReplacement{row[0]->data(Qt::DisplayRole).toString(),
                            row[1]->data(Qt::CheckStateRole).toBool(),
                            row[3]->data(Qt::CheckStateRole).toBool(),
                            row[4]->data(Qt::DisplayRole).toString(),
                            row[2]->data(Qt::CheckStateRole).toBool()};
}

// turns a row in the model into a vector item
void InputReplacementModel::getRowFromItem(const InputReplacement &item,
                                           std::vector<QStandardItem *> &row)
{
    setStringItem(row[0], item.getPattern());
    setBoolItem(row[1], item.isRegex());
    setBoolItem(row[3], item.isEnabled());
    setStringItem(row[4], item.getReplace());
    setBoolItem(row[2], item.isCaseSensitive());
}

}  // namespace chatterino
