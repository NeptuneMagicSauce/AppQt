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
    QToolButton* button = nullptr;
    bool state = false;
} impl_s;

SettingsPane::SettingsPane(QWidget* parent) :
    // QWidget(parent)
    // QDockWidget(parent)
    QDialog(parent)
{
    Utils::assertSingleton(typeid(*this)); // have impl be member if not singleton
    impl_s.parent = parent;

    resize(200, 200);

    // // as QDockWidget ->
    // hide();
    // setWindowTitle("Settings");
    // setFeatures(QDockWidget::NoDockWidgetFeatures);

    // as QDialog ->
    setWindowFlags(
        Qt::FramelessWindowHint |
        // Qt::Tool // Tool = Popup | Dialog
        Qt::Popup // auto-close when click away
        // Qt::Dialog // is another window, takes focus away from parent
        // Qt::Widget
        );
    setModal(false);

    auto& action = impl_s.action;
    action = new QAction(Labels::settings);
    action->setCheckable(true);
    action->setToolTip("Settings");
    action->setShortcut(Qt::Key_F2);
    // MacOS: prefer standard shortcut Preferences, it does not exist on Windows
    // QKeySequence::Preferences);
    QObject::connect(action, &QAction::toggled, [this] (bool checked) {
        if (checked) { show(); }
        else { hide(); }
    });

    // TODO update tool bar button state: on hideEvent() ?
    // TODO parent window management is broken: resize, close ...
    // TODO parent window does not catch actions: F1, F2 ...
    // TODO check if my height is bigger than parent height

}

QAction* SettingsPane::action(void) const
{
    return impl_s.action;
}

void SettingsPane::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
    Assert(impl_s.button);
    impl_s.button->setDown(false);
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

    auto& button = impl_s.button;
    if (button == nullptr)
    {
        for (auto* w: impl_s.action->associatedWidgets())
        {
            auto*b = dynamic_cast<QToolButton*>(w);
            if (b != nullptr)
            {
                button = b;
                break;
            }
        }
    }
}
