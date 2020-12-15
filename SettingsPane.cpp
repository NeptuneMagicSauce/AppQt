#include "SettingsPane.hpp"

#include <QResizeEvent>
#include <QDebug>
#include <QToolButton>
#include <QTimer>
#include <QSlider>
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
            raise();
            parent_geometry = { 0, 0, 0, 0 };
            watch_parent_timer.start();
        } else {
            watch_parent_timer.stop();
        }
    });

    setFixedWidth(200);
    setLayout(new QVBoxLayout);
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    hide();

    QObject::connect(&watch_parent_timer, &QTimer::timeout, [this] () {
        auto current_parent_geometry = dynamic_cast<QWidget*>(this->parent())->geometry();
        if (current_parent_geometry.width() != parent_geometry.width())
        {
            move(current_parent_geometry.width() - width(), 0);
            parent_geometry = current_parent_geometry;
        }
    });
}

QAction* SettingsPane::action(const QString& change_label)
{
    if (change_label.size())
    {
        m_action.setText(change_label);
    }
    return &m_action;
}

SettingsPane::Widgets SettingsPane::beginCreate(QString name, QString longest_value)
{
    auto widget = new QGroupBox(name);
    auto sub_layout = new QHBoxLayout;
    auto value_label = new QLabel;
    widget->setLayout(sub_layout);
    sub_layout->addWidget(value_label);
    value_label->setAlignment(Qt::AlignCenter);
    value_label->setText(longest_value);
    value_label->setMinimumWidth(value_label->sizeHint().width());
    return { widget, sub_layout, value_label };
}


void SettingsPane::create(QString name, int value, QPoint range, int step, Callback callback)
{
    auto [ widget, sub_layout, value_label ] = beginCreate(name, QString::number(range.y()));

    auto slider = new QSlider;
    sub_layout->addWidget(slider);
    slider->setTracking(false);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(range.x(), range.y());
    slider->setValue(value);
    slider->setSingleStep(step);
    slider->setTickInterval(step);
    slider->setTickPosition(QSlider::TicksBelow);
    value_label->setText(QString::number(value));
    QObject::connect(slider, &QSlider::sliderMoved, [value_label] (int value) {
        value_label->setText(QString::number(value));
    });
    QObject::connect(slider, &QSlider::valueChanged,
                     [this, value_label, callback] (int value) {
                         value_label->setText(QString::number(value));
                         callback(value);
                     });
    endCreate(widget);
}

void SettingsPane::endCreate(QWidget* widget)
{
    Assert(layout());
    layout()->addWidget(widget);
    adjustSize();
    auto parent_widget = dynamic_cast<QWidget*>(parent());
    auto parent_min_size = parent_widget->minimumSize();
    parent_widget->setMinimumSize(
        std::max(width(), parent_min_size.width()),
        std::max(height(), parent_min_size.height())
        );
}
