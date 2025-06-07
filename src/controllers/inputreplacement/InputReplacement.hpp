#pragma once

#include "util/RapidjsonHelpers.hpp"

#include <pajlada/serialize.hpp>
#include <QRegularExpression>
#include <QString>

namespace chatterino {

class InputReplacement
{
public:
    InputReplacement(const QString &pattern, bool isRegex, bool isEnabled,
                     const QString &replace, bool isCaseSensitive);

    InputReplacement();

    bool operator==(const InputReplacement &other) const;

    const QString &getPattern() const;

    bool isRegex() const;

    bool isRegexValid() const;

    bool isEnabled() const;

    const QRegularExpression &getRegex() const;

    const QString &getReplace() const;

    bool isCaseSensitive() const;

    Qt::CaseSensitivity caseSensitivity() const;

    static InputReplacement createEmpty();

private:
    QString pattern_;
    bool isRegex_;
    bool isEnabled_;
    QRegularExpression regex_;
    QString replace_;
    bool isCaseSensitive_;
};

}  // namespace chatterino

namespace pajlada {

template <>
struct Serialize<chatterino::InputReplacement> {
    static rapidjson::Value get(const chatterino::InputReplacement &value,
                                rapidjson::Document::AllocatorType &a)
    {
        rapidjson::Value ret(rapidjson::kObjectType);

        chatterino::rj::set(ret, "pattern", value.getPattern(), a);
        chatterino::rj::set(ret, "regex", value.isRegex(), a);
        chatterino::rj::set(ret, "isEnabled", value.isEnabled(), a);
        chatterino::rj::set(ret, "replaceWith", value.getReplace(), a);
        chatterino::rj::set(ret, "caseSensitive", value.isCaseSensitive(), a);

        return ret;
    }
};

template <>
struct Deserialize<chatterino::InputReplacement> {
    static chatterino::InputReplacement get(const rapidjson::Value &value,
                                            bool *error = nullptr)
    {
        if (!value.IsObject())
        {
            PAJLADA_REPORT_ERROR(error)
            return chatterino::InputReplacement::createEmpty();
        }

        QString _pattern;
        bool _isRegex = false;
        bool _isEnabled = true;
        QString _replace;
        bool _caseSens = true;

        chatterino::rj::getSafe(value, "pattern", _pattern);
        chatterino::rj::getSafe(value, "regex", _isRegex);
        chatterino::rj::getSafe(value, "isEnabled", _isEnabled);
        chatterino::rj::getSafe(value, "replaceWith", _replace);
        chatterino::rj::getSafe(value, "caseSensitive", _caseSens);

        return chatterino::InputReplacement(_pattern, _isRegex, _isEnabled,
                                            _replace, _caseSens);
    }
};

}  // namespace pajlada
