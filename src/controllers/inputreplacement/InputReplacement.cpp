#include "controllers/inputreplacement/InputReplacement.hpp"

namespace chatterino {

InputReplacement::InputReplacement(const QString &pattern, bool isRegex,
                                   bool isEnabled, const QString &replace,
                                   bool isCaseSensitive)
    : pattern_(pattern)
    , isRegex_(isRegex)
    , isEnabled_(isEnabled)
    , regex_(pattern)
    , replace_(replace)
    , isCaseSensitive_(isCaseSensitive)
{
    if (this->isCaseSensitive_)
    {
        regex_.setPatternOptions(
            QRegularExpression::UseUnicodePropertiesOption);
    }
    else
    {
        regex_.setPatternOptions(
            QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::UseUnicodePropertiesOption);
    }
}

InputReplacement::InputReplacement()
    : InputReplacement("", true, true, "", true)
{
}

bool InputReplacement::operator==(const InputReplacement &other) const
{
    return std::tie(this->pattern_, this->isRegex_, this->isEnabled_,
                    this->replace_, this->isCaseSensitive_) ==
           std::tie(other.pattern_, other.isRegex_, other.isEnabled_,
                    other.replace_, other.isCaseSensitive_);
}

const QString &InputReplacement::getPattern() const
{
    return this->pattern_;
}

bool InputReplacement::isRegex() const
{
    return this->isRegex_;
}

bool InputReplacement::isRegexValid() const
{
    return this->regex_.isValid();
}

bool InputReplacement::isEnabled() const
{
    return this->isEnabled_;
}

const QRegularExpression &InputReplacement::getRegex() const
{
    return this->regex_;
}

const QString &InputReplacement::getReplace() const
{
    return this->replace_;
}

bool InputReplacement::isCaseSensitive() const
{
    return this->isCaseSensitive_;
}

Qt::CaseSensitivity InputReplacement::caseSensitivity() const
{
    return this->isCaseSensitive_ ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

InputReplacement InputReplacement::createEmpty()
{
    return InputReplacement("Your Pattern", true, true, "Your Replacement",
                            true);
}

}  // namespace chatterino
