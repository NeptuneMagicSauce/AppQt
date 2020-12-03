#include "Gui.hpp"

#include <QToolButton>
#include <QResizeEvent>

#include "Labels.hpp"

using namespace Minus;

Gui::Gui(const int& width, const int& height) :
    frame(width, height)
{
    main_window.setCentralWidget(&frame);
    main_window.setWindowTitle("Super Minus");
    main_window.show();
    createToolBar();
}

void Gui::createToolBar(void)
{
    tool_bar.setFloatable(false);
    tool_bar.setMovable(false);
    auto* spacer_left = new QWidget;
    spacer_left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* spacer_right = new QWidget;
    spacer_right->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tool_bar.addWidget(spacer_left);
    auto* action_reset = tool_bar.addAction(
        Labels::reset,
        [this]() {
            // reset(this->width, this->height);
            frame.reset();
            emit reset();
        });
    tool_bar.addWidget(spacer_right);
    tool_bar.setToolButtonStyle(Qt::ToolButtonTextOnly);
    action_reset->setShortcut(QKeySequence::Refresh);
    action_reset->setToolTip("Reset (" + action_reset->shortcut().toString() + ")");
    auto font = action_reset->font();
    font.setPointSize(16);
    action_reset->setFont(font);
    for (auto* w: action_reset->associatedWidgets())
    {
        auto* reset_button = dynamic_cast<QToolButton*>(w);
        if (reset_button != nullptr)
        {
            reset_button->setAutoRaise(false);
        }
    }
    tool_bar.setContextMenuPolicy(Qt::PreventContextMenu);
    main_window.addToolBar(Qt::TopToolBarArea, &tool_bar);
}

void Gui::resizeEvent(void)
{
    // emit resize event
    QResizeEvent e { frame.size(), QSize(0, 0) };
    frame.resizeEvent(&e);
}
