#include "controllers/inputreplacement/InputReplacementController.hpp"

#include "Application.hpp"
#include "controllers/inputreplacement/InputReplacement.hpp"
#include "controllers/inputreplacement/InputReplacementModel.hpp"
#include "singletons/Paths.hpp"
#include "singletons/Settings.hpp"
#include "util/CombinePath.hpp"

#include <QString>

namespace {

/**
  * Computes (only) the replacement of @a match in @a source.
  * The parts before and after the match in @a source are ignored.
  *
  * Occurrences of \b{\\1}, \b{\\2}, ..., in @a replacement are replaced
  * with the string captured by the corresponding capturing group.
  * This function should only be used if the regex contains capturing groups.
  * 
  * Since Qt doesn't provide a way of replacing a single match with some replacement
  * while supporting both capturing groups and lookahead/-behind in the regex,
  * this is included here. It's essentially the implementation of 
  * QString::replace(const QRegularExpression &, const QString &).
  * @see https://github.com/qt/qtbase/blob/97bb0ecfe628b5bb78e798563212adf02129c6f6/src/corelib/text/qstring.cpp#L4594-L4703
  */
QString makeRegexReplacement(QStringView source,
                             const QRegularExpression &regex,
                             const QRegularExpressionMatch &match,
                             const QString &replacement)
{
    using SizeType = QString::size_type;
    struct QStringCapture {
        SizeType pos;
        SizeType len;
        int captureNumber;
    };

    qsizetype numCaptures = regex.captureCount();

    // 1. build the backreferences list, holding where the backreferences
    //    are in the replacement string
    QVarLengthArray<QStringCapture> backReferences;

    SizeType replacementLength = replacement.size();
    for (SizeType i = 0; i < replacementLength - 1; i++)
    {
        if (replacement[i] != u'\\')
        {
            continue;
        }

        int no = replacement[i + 1].digitValue();
        if (no <= 0 || no > numCaptures)
        {
            continue;
        }

        QStringCapture backReference{.pos = i, .len = 2};

        if (i < replacementLength - 2)
        {
            int secondDigit = replacement[i + 2].digitValue();
            if (secondDigit != -1 && ((no * 10) + secondDigit) <= numCaptures)
            {
                no = (no * 10) + secondDigit;
                ++backReference.len;
            }
        }

        backReference.captureNumber = no;
        backReferences.append(backReference);
    }

    // 2. iterate on the matches.
    //    For every match, copy the replacement string in chunks
    //    with the proper replacements for the backreferences

    // length of the new string, with all the replacements
    SizeType newLength = 0;
    QVarLengthArray<QStringView> chunks;
    QStringView replacementView{replacement};

    // Initially: empty, as we only care about the replacement
    SizeType len = 0;
    SizeType lastEnd = 0;
    for (const QStringCapture &backReference : std::as_const(backReferences))
    {
        // part of "replacement" before the backreference
        len = backReference.pos - lastEnd;
        if (len > 0)
        {
            chunks << replacementView.mid(lastEnd, len);
            newLength += len;
        }

        // backreference itself
        len = match.capturedLength(backReference.captureNumber);
        if (len > 0)
        {
            chunks << source.mid(
                match.capturedStart(backReference.captureNumber), len);
            newLength += len;
        }

        lastEnd = backReference.pos + backReference.len;
    }

    // add the last part of the replacement string
    len = replacementView.size() - lastEnd;
    if (len > 0)
    {
        chunks << replacementView.mid(lastEnd, len);
        newLength += len;
    }

    // 3. assemble the chunks together
    QString dst;
    dst.reserve(newLength);
    for (const QStringView &chunk : std::as_const(chunks))
    {
        dst += chunk;
    }
    return dst;
}

}  // namespace

namespace chatterino {

InputReplacementController::InputReplacementController(const Paths &paths)
{
    // Update input replacements map when the vector of input replacements has been updated
    auto addFirstMatchToMap = [this](auto args) {
        this->inputReplacementsMap_.remove(args.item.getPattern());

        for (const InputReplacement &inpRepl : this->items)
        {
            if (inpRepl.getPattern() == args.item.getPattern())
            {
                this->inputReplacementsMap_[inpRepl.getPattern()] = inpRepl;
                break;
            }
        }
    };
    // We can safely ignore these signal connections since items will be destroyed
    // before InputReplacementController
    std::ignore = this->items.itemInserted.connect(addFirstMatchToMap);
    std::ignore = this->items.itemRemoved.connect(addFirstMatchToMap);

    // Initialize setting manager for inputreplacements.json
    auto path = combinePath(paths.settingsDirectory, "inputreplacements.json");
    this->sm_ = std::make_shared<pajlada::Settings::SettingManager>();
    this->sm_->setPath(qPrintable(path));
    this->sm_->setBackupEnabled(true);
    this->sm_->setBackupSlots(9);

    // Delayed initialization of the setting storing all input replacements
    this->inputReplacementSetting_.reset(
        new pajlada::Settings::Setting<std::vector<InputReplacement>>(
            "/inputreplacements", this->sm_));

    // Update the setting when the vector of input replacements has been updated (most
    // likely from the settings dialog)
    std::ignore = this->items.delayedItemsChanged.connect([this] {
        this->inputReplacementSetting_->setValue(this->items.raw());
    });

    // Load input replacements from inputreplacements.json
    this->sm_->load();

    // Add loaded input replacements to our vector of input replacements (which will update the map
    // of input replacements)
    for (const auto &inputReplacement :
         this->inputReplacementSetting_->getValue())
    {
        this->items.append(inputReplacement);
    }
}

void InputReplacementController::save()
{
    this->sm_->save();
}

InputReplacementModel *InputReplacementController::createModel(QObject *parent)
{
    InputReplacementModel *model = new InputReplacementModel(parent);
    model->initialize(&this->items);

    return model;
}

void InputReplacementController::replaceCharactersInMessage(QString &msg)
{
    using SizeType = QString::size_type;

    if (!getSettings()->globallyEnableInputReplacements.getValue() ||
        msg.isEmpty())
    {
        return;
    }

    for (const auto &inpRepl : this->items)
    {
        if (!inpRepl.isEnabled())
        {
            continue;
        }

        const auto &pattern = inpRepl.getPattern();
        if (pattern.isEmpty())
        {
            continue;
        }

        if (inpRepl.isRegex())
        {
            const auto &regex = inpRepl.getRegex();
            if (!regex.isValid())
            {
                continue;
            }

            QRegularExpressionMatch match;
            size_t iterations = 0;
            SizeType from = 0;
            while ((from = msg.indexOf(regex, from, &match)) != -1)
            {
                auto replacement = inpRepl.getReplace();
                if (regex.captureCount() > 0)
                {
                    replacement =
                        makeRegexReplacement(msg, regex, match, replacement);
                }

                msg.replace(from, match.capturedLength(), replacement);

                from += replacement.length();
                iterations++;
                if (iterations >= 128)
                {
                    return;
                }
            }

            continue;
        }

        SizeType from = 0;
        while ((from = msg.indexOf(pattern, from, inpRepl.caseSensitivity())) !=
               -1)
        {
            msg.replace(from, pattern.length(), inpRepl.getReplace());
            from += inpRepl.getReplace().length();
        }
    }
}

}  // namespace chatterino
