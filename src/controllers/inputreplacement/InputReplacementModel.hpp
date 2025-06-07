#pragma once

#include "common/SignalVectorModel.hpp"
#include "controllers/inputreplacement/InputReplacement.hpp"

#include <QObject>

namespace chatterino {

class InputReplacement;

class InputReplacementModel : public SignalVectorModel<InputReplacement>
{
public:
    explicit InputReplacementModel(QObject *parent);

protected:
    // turn a vector item into a model row
    InputReplacement getItemFromRow(std::vector<QStandardItem *> &row,
                                    const InputReplacement &original) override;

    // turns a row in the model into a vector item
    void getRowFromItem(const InputReplacement &item,
                        std::vector<QStandardItem *> &row) override;
};

}  // namespace chatterino
