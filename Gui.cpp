#include "Gui.hpp"

#include <QDebug>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QResizeEvent>
#include <QScreen>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

#include "Utils.hpp"
#include "Labels.hpp"

using namespace Minus;
using namespace Utils;

class EventFilterFirstShow: public QObject
{
public:
    using Callback = std::function<void()>;
    Callback callback;
    EventFilterFirstShow(Callback callback=nullptr) : callback(callback) { }
protected:
    bool has_run = false;
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        // qDebug() << event->type();
        if (has_run == false && event->type() == QEvent::Show)
        {
            has_run = true;
            if (callback != nullptr)
            {
                callback();
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

class GuiImpl
{
public:
    GuiImpl(Gui& gui);
private:
    Gui& gui;
    EventFilterFirstShow filter;

    QMainWindow main_window;
    QToolBar tool_bar;

    void formatAction(QAction* action)
    {
        action->setToolTip(
            action->toolTip() + " (" +
            action->shortcut().toString() + ")");
        auto font = action->font();
        font.setPointSize(16);
        action->setFont(font);
        for (auto* w: action->associatedWidgets())
        {
            auto* button = dynamic_cast<QToolButton*>(w);
            if (button != nullptr)
            {
                button->setAutoRaise(false);
            }
        }
    }

    void addButton(
        std::function<void()> callback,
        const QString& label,
        const QString& tool_tip,
        const QKeySequence& shortcut)
    {
        auto* action = tool_bar.addAction(label, callback);
        action->setToolTip(tool_tip);
        action->setShortcut(shortcut);
        formatAction(action);
    }

    void setInitialWindowSize(void)
    {
        auto* window = tool_bar.window();
        if (!window) { return; }
        auto* screen = window->screen();
        if (!screen) { return; }
        auto screen_geom = screen->availableVirtualGeometry();
        // qDebug() << screen_geom;

        int width = gui.frame.width * Frame::InitialCellSize;
        // tool_bar.height() does not have correct value
        // before first Show event
        int height = tool_bar.height() + gui.frame.height * Frame::InitialCellSize;

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
        tool_bar.removeEventFilter(&filter);
    }
};

Gui::Gui(const int& width, const int& height) :
    frame(width, height),
    settings(&frame)
{
    Utils::assertSingleton(typeid(*this));
    new GuiImpl(*this);
}

GuiImpl::GuiImpl(Gui& gui) :
    gui(gui)
{
    main_window.setCentralWidget(&gui.frame);
    main_window.setWindowTitle("Super Minus");
    main_window.show();

    filter.callback = [this] () { setInitialWindowSize(); };
    tool_bar.installEventFilter(&filter);
    tool_bar.setFloatable(false);
    tool_bar.setMovable(false);

    auto* spacer_left = new QWidget;
    spacer_left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* spacer_right = new QWidget;
    spacer_right->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tool_bar.addWidget(spacer_left);
    // TODO button reset is not perfectly centered
    addButton(
        [&gui]() {
            emit gui.reset_signal(gui.frame.width, gui.frame.height);
        },
        Labels::reset,
        "Reset",
        QKeySequence::Refresh);
    tool_bar.addWidget(spacer_right);

    auto* settings_action = gui.settings.action(Labels::settings);
    tool_bar.addAction(settings_action);
    formatAction(settings_action);

    auto mine_range = QPoint { 5, 100 };
    auto setting_id_width = gui.settings.registerInt(
        "Width",
        gui.frame.width,
        mine_range);
    auto setting_id_height = gui.settings.registerInt(
        "Height",
        gui.frame.height,
        mine_range);
    QObject::connect(
        &gui.settings,
        &SettingsPane::intChanged,

        [&gui, setting_id_width, setting_id_height] (int id, int value) {
            auto width = id == setting_id_width ? value : gui.frame.width;
            auto height = id == setting_id_height ? value : gui.frame.height;
            emit gui.reset_signal(width, height);
            // TODO BUG missing call to resize event because font is not scaled
            // TODO BUG here: I can see old zombie CellWidgets after reset
        });

    tool_bar.setToolButtonStyle(Qt::ToolButtonTextOnly);
    tool_bar.setContextMenuPolicy(Qt::PreventContextMenu);
    main_window.addToolBar(Qt::TopToolBarArea, &tool_bar);

}

void Gui::reset(void)
{
    frame.reset();
    for (int x=0; x<frame.width; ++x)
    {
        for (int y=0; y<frame.height; ++y)
        {
            frame.addCell(y, x);
        }
    }
}

void Gui::resizeEvent(void)
{
    // trigger resize event in Frame
    auto e = QResizeEvent { frame.size(), QSize(0, 0) };
    frame.resizeEvent(&e);
}
