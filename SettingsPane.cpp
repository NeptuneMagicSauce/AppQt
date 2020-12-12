#include "SettingsPane.hpp"

#include <QResizeEvent>
#include <QDebug>
#include <QToolButton>
#include <QTimer>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QVBoxLayout>

#include "Utils.hpp"
#include "Labels.hpp"

using namespace Utils;

SettingsPane::SettingsPane(QWidget* parent) :
    QFrame(parent),
    m_action("Settings")
{
    Assert(parent != nullptr);

    m_action.setCheckable(true);
    m_action.setToolTip("Settings");
    m_action.setShortcut(Qt::Key_F2);
    // MacOS: prefer standard shortcut Preferences, it does not exist on Windows
    // QKeySequence::Preferences);
    QObject::connect(&m_action, &QAction::toggled, [this] (bool checked) {
        setVisible(checked);
        if (checked)
        {
            setFocus(Qt::OtherFocusReason); // so that we receive focusOutEvent
        }
    });
    // TODO check if my height is taller than parent height

    setLayout(new QVBoxLayout);
    setFrameShape(QFrame::StyledPanel);

    setAutoFillBackground(true);
    raise();
    hide();
}

QAction* SettingsPane::action(const QString& change_label)
{
    if (change_label.size())
    {
        m_action.setText(change_label);
    }
    return &m_action;
}

void SettingsPane::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
    // TODO positioning is broken on resize parent !
    auto p = dynamic_cast<QWidget*>(parent());
    move({ p->width() - width(), 0 });
}

void SettingsPane::focusOutEvent(QFocusEvent *event)
{
    QFrame::focusOutEvent(event);
    m_action.setChecked(false);
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
    auto index = next_setting_index;
    QObject::connect(slider, &QSlider::valueChanged,
                     [this, value_label, index] (int value) {
                         value_label->setText(QString::number(value));
                         emit integerChanged(index, value);
                     });
    QObject::connect(slider, &QSlider::sliderMoved, [value_label] (int value) {
        value_label->setText(QString::number(value));
    });
    layout()->addWidget(widget);
    resize(200, layout()->count() * 100);
    return next_setting_index++;
}
