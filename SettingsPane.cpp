#include "SettingsPane.hpp"

using namespace Minus;

class SettingsPaneImpl
{
public:
    SettingsPane& panel;
    SettingsPaneImpl(SettingsPane& panel);
};

SettingsPane::SettingsPane()
{
    new SettingsPaneImpl(*this);
}

SettingsPaneImpl::SettingsPaneImpl(SettingsPane& panel) :
    panel(panel)
{
    panel.hide();
    panel.setMaximumSize(200, 0);
}
