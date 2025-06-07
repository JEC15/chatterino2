#include "widgets/settingspages/InputReplacementPage.hpp"

#include "Application.hpp"
#include "controllers/inputreplacement/InputReplacement.hpp"
#include "controllers/inputreplacement/InputReplacementController.hpp"
#include "controllers/inputreplacement/InputReplacementModel.hpp"
#include "singletons/Settings.hpp"
#include "util/LayoutCreator.hpp"
#include "widgets/helper/EditableModelView.hpp"

#include <QColor>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

using namespace chatterino;

void checkPatternDuplicates(EditableModelView *view,
                            QLabel *duplicatePatternWarning)
{
    bool foundDuplicatePatternTrigger = false;

    // Maps characters to replace to model row indices
    std::unordered_map<QString, std::vector<int>> replacements;

    for (int i = 0; i < view->getModel()->rowCount(); i++)
    {
        QString pattern = view->getModel()->index(i, 0).data().toString();
        replacements[pattern].push_back(i);
    }

    for (const auto &[pattern, rowIndices] : replacements)
    {
        assert(!rowIndices.empty());

        if (rowIndices.size() > 1)
        {
            foundDuplicatePatternTrigger = true;

            for (const auto &rowIndex : rowIndices)
            {
                view->getModel()->setData(view->getModel()->index(rowIndex, 0),
                                          QColor("yellow"), Qt::ForegroundRole);
            }
        }
        else
        {
            view->getModel()->setData(view->getModel()->index(rowIndices[0], 0),
                                      QColor("white"), Qt::ForegroundRole);
        }
    }

    if (foundDuplicatePatternTrigger)
    {
        duplicatePatternWarning->show();
    }
    else
    {
        duplicatePatternWarning->hide();
    }
}

}  // namespace

namespace chatterino {

InputReplacementPage::InputReplacementPage()
{
    LayoutCreator<InputReplacementPage> layoutCreator(this);
    auto layout = layoutCreator.setLayoutType<QVBoxLayout>();

    QLabel *description =
        layout
            .emplace<QLabel>(
                "Replace characters in your message before it is "
                "sent to the servers. Replacement will not be performed if "
                "your message is a twich or chatterino command.\nReplacements "
                "are processed in order, from top to bottom.")
            .getElement();
    description->setWordWrap(true);

    auto *enableInputReplacementCheckBox =
        this->createCheckBox("Enable input replacements",
                             getSettings()->globallyEnableInputReplacements);

    layout.append(enableInputReplacementCheckBox);

    auto *view = layout
                     .emplace<EditableModelView>(
                         getApp()->getInputReplacements()->createModel(nullptr))
                     .getElement();

    view->setTitles(
        {"Pattern", "Regex", "Case-sensitive", "Enabled", "Replacement"});
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Interactive);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        3, QHeaderView::ResizeToContents);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        4, QHeaderView::Stretch);
    view->getTableView()->horizontalHeader()->resizeSection(0, 300);

    // We can safely ignore this signal connection since we own the view
    std::ignore = view->addButtonPressed.connect([] {
        getApp()->getInputReplacements()->items.append(InputReplacement{
            "Your pattern", true, true, "Your replacement", true});
    });

    auto *duplicatePatternWarning =
        layout.emplace<QLabel>("Duplicate pattern found.").getElement();
    duplicatePatternWarning->setStyleSheet("color: yellow");

    // NOTE: These signals mean that the duplicate check happens in the middle of a row being moved, where he index can be wrong.
    // This should be reconsidered, or potentially changed in the signalvectormodel. Or maybe we rely on a SignalVectorModel signal instead
    QObject::connect(view->getModel(), &QAbstractItemModel::rowsInserted, this,
                     [view, duplicatePatternWarning]() {
                         checkPatternDuplicates(view, duplicatePatternWarning);
                     });

    QObject::connect(view->getModel(), &QAbstractItemModel::rowsRemoved, this,
                     [view, duplicatePatternWarning]() {
                         checkPatternDuplicates(view, duplicatePatternWarning);
                     });

    QObject::connect(
        view->getModel(), &QAbstractItemModel::dataChanged, this,
        [view, duplicatePatternWarning](const QModelIndex &topLeft,
                                        const QModelIndex &bottomRight,
                                        const QVector<int> &roles) {
            (void)topLeft;
            (void)bottomRight;
            if (roles.contains(Qt::EditRole))
            {
                checkPatternDuplicates(view, duplicatePatternWarning);
            }
        });

    checkPatternDuplicates(view, duplicatePatternWarning);
}

}  // namespace chatterino
