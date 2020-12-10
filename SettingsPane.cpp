#include "SettingsPane.hpp"

#include <QResizeEvent>
#include <QDebug>
#include <QToolButton>
#include <QTimer>

#include "Utils.hpp"
#include "Labels.hpp"

using namespace Minus;

class SettingsImpl
{
public:
    QWidget* parent = nullptr;
    QAction* action = nullptr;
} impl_s;

SettingsPane::SettingsPane(QWidget* parent) :
    QDialog(parent)
{
    Utils::assertSingleton(typeid(*this)); // have impl be member if not singleton
    impl_s.parent = parent;

    resize(200, 200);

    // as QDialog ->
    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Popup // auto-close when click away
        // Qt::Dialog // is another window, takes focus away from parent
        );
    setModal(false);

    auto& action = impl_s.action;
    action = new QAction(Labels::settings);
    action->setCheckable(true);
    action->setToolTip("Settings");
    action->setShortcut(Qt::Key_F2);
    // MacOS: prefer standard shortcut Preferences, it does not exist on Windows
    // QKeySequence::Preferences);
    QObject::connect(action, &QAction::triggered, [this] (bool checked) {
        if (checked)
        {
            show();
        }
        // no need to test checked == false
        // because clicking away from the dialog (on the action) hides it
    });
    // TODO check if my height is taller than parent height
}

QAction* SettingsPane::action(void) const
{
    return impl_s.action;
}

void SettingsPane::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    // needs to setChecked(false) on hide from clicking away
    // with delay in case we click away on the action button
    QTimer::singleShot(200, [] () {
        impl_s.action->setChecked(false);
    });
}

void SettingsPane::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // anchor in top right corner of parent
    auto parent = impl_s.parent;
    setGeometry({parent->mapToGlobal({parent->width() - width(), 0}), size()});
    // auto top_left = parent->mapToGlobal({0, 0});
    // top_left.setX(top_left.x() + parent->width() - width());
    // setGeometry({ top_left, size() });
}
