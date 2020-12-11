#include "SettingsPane.hpp"

#include <QResizeEvent>
#include <QDebug>
#include <QToolButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>

#include "Utils.hpp"
#include "Labels.hpp"

using namespace Utils;

class SettingsImpl
{
public:
    QWidget* parent = nullptr;
    QAction* action = nullptr;
    QVBoxLayout* layout = nullptr;
    int index = 0;
} impl_s;

SettingsPane::SettingsPane(QWidget* parent) :
    QDialog(parent)
{
    Utils::assertSingleton(typeid(*this)); // have impl be member if not singleton
    impl_s.parent = parent;

    // resize(200, 200);

    // as QDialog ->
    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Popup // auto-close when click away
        // Qt::Dialog // is another window, takes focus away from parent
        );
    setModal(false);

    auto& action = impl_s.action;
    action = new QAction("Settings");
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

    impl_s.layout = new QVBoxLayout;
    setLayout(impl_s.layout);
}

QAction* SettingsPane::action(const QString& label)
{
    if (label.size())
    {
        impl_s.action->setText(label);
    }
    return impl_s.action;
}

void SettingsPane::hideEvent(QHideEvent *event)
{
    // TODO fix dirty hack : timer with magic value
    QDialog::hideEvent(event);
    // needs to setChecked(false) on hide from clicking away
    // with delay in case we click away on the action button
    QTimer::singleShot(200, [] () {
        impl_s.action->setChecked(false);
    });
}

void SettingsPane::showEvent(QShowEvent *event)
{
    // TODO positioning is broken ...
    // but we are hidden on any click away, such as move/resize window
    // moving the window with the keyboard does not hide us, show its broken
    // -> do same way as progress bar in Frame

    QDialog::showEvent(event);
    // anchor in top right corner of parent
    auto parent = impl_s.parent;
    setGeometry({parent->mapToGlobal({parent->width() - width(), 0}), size()});
    // auto top_left = parent->mapToGlobal({0, 0});
    // top_left.setX(top_left.x() + parent->width() - width());
    // setGeometry({ top_left, size() });
}

int SettingsPane::registerInt(QString label, int value, QPoint range)
{
    auto widget = new QGroupBox(label);
    // auto layout = new QHBoxLayout;
    // auto sub_widget = new QWidget;
    auto sub_layout = new QVBoxLayout;
    auto slider = new QSlider;
    auto value_label = new QLabel;
    widget->setLayout(sub_layout);
    // sub_widget->setLayout(sub_layout);
    // layout->addWidget(new QLabel(label));
    // layout->addWidget(sub_widget);
    sub_layout->addWidget(slider);
    sub_layout->addWidget(value_label);
    value_label->setAlignment(Qt::AlignCenter);
    slider->setTracking(false);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(range.x(), range.y());
    slider->setValue(value);
    value_label->setText(QString::number(value));
    auto index = impl_s.index;
    QObject::connect(slider, &QSlider::valueChanged,
                     [this, value_label, index] (int value) {
                         value_label->setText(QString::number(value));
                         emit intChanged(index, value);
                     });
    QObject::connect(slider, &QSlider::sliderMoved, [value_label] (int value) {
        value_label->setText(QString::number(value));
    });
    impl_s.layout->addWidget(widget);
    return impl_s.index++;
}
