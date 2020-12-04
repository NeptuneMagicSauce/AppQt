#include "Gui.hpp"

#include <QToolButton>
#include <QResizeEvent>
#include <QScreen>
#include <QDebug>

#include "Labels.hpp"

using namespace Minus;

class MainEventFilter: public QObject
{
public:
    MainEventFilter(Frame& frame, QToolBar& tool_bar) :
        frame(frame),
        tool_bar(tool_bar)
    { }
protected:
    Frame& frame;
    QToolBar& tool_bar;
    bool first_run { true };
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        // qDebug() << event->type() << tool_bar.height();
        if (first_run && event->type() == QEvent::Show)
        {
            first_run = false;
            setInitialWindowSize();
            tool_bar.removeEventFilter(this);
        }
        return QObject::eventFilter(obj, event);
    }

    void setInitialWindowSize(void)
    {
        auto* window = tool_bar.window();
        if (!window) { return; }
        auto* screen = window->screen();
        if (!screen) { return; }
        auto screen_geom = screen->availableVirtualGeometry();
        // qDebug() << screen_geom;

        int width = frame.width * Frame::InitialCellSize;
        int height = tool_bar.height() + frame.height * Frame::InitialCellSize;

        if (width >= screen_geom.width() ||
            height >= screen_geom.height())
        {
            // window->move(screen_geom.topLeft());
            // window->resize(screen_geom.width(), screen_geom.height());
            window->showMaximized();
        } else {
            auto center = screen_geom.center();
            window->setGeometry(QRect(
                center.x() - width / 2,
                center.y() - height / 2,
                width,
                height
                ));
        }
    }
};

Gui::Gui(const int& width, const int& height) :
    frame(width, height)
{
    main_window.setCentralWidget(&frame);
    main_window.setWindowTitle("Super Minus");
    main_window.show();
    tool_bar.installEventFilter(new MainEventFilter(frame, tool_bar));
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
