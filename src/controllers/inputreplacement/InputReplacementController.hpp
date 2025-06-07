#pragma once

#include "common/SignalVector.hpp"
#include "controllers/inputreplacement/InputReplacement.hpp"

#include <pajlada/settings.hpp>
#include <QMap>

namespace chatterino {

class Settings;
class Paths;

class InputReplacement;
class InputReplacementModel;

class InputReplacementController final
{
public:
    SignalVector<InputReplacement> items;

    InputReplacementController(const Paths &paths);

    void save();

    InputReplacementModel *createModel(QObject *parent);

    void replaceCharactersInMessage(QString &msg);

private:
    void load(Paths &paths);

    QMap<QString, InputReplacement> inputReplacementsMap_;

    std::shared_ptr<pajlada::Settings::SettingManager> sm_;
    // Because the setting manager is not initialized until the initialize
    // function is called (and not in the constructor), we have to
    // late-initialize the setting, which is why we're storing it as a
    // unique_ptr
    std::unique_ptr<pajlada::Settings::Setting<std::vector<InputReplacement>>>
        inputReplacementSetting_;
};

}  // namespace chatterino
