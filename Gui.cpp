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
#include <QTime>

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

    static constexpr int MaxWidth = 50;
    static constexpr int MaxHeight = 40;

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
        // TODO BUG setInitialWindowSize : puts title bar out of screen
        auto* window = tool_bar.window();
        if (!window) { return; }
        auto* screen = window->screen();
        if (!screen) { return; }
        auto screen_geom = screen->availableVirtualGeometry();
        // qDebug() << screen_geom;

        const auto initial_cell_size = 40;
        int width = gui.frame.width * initial_cell_size;
        // tool_bar.height() does not have correct value
        // before first Show event
        int height = tool_bar.height() + gui.frame.height * initial_cell_size;

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
    frame(width, height, GuiImpl::MaxWidth, GuiImpl::MaxHeight),
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

    auto setting_id_width = gui.settings.registerInt(
        "Width",
        gui.frame.width,
        { 5, GuiImpl::MaxWidth });
    auto setting_id_height = gui.settings.registerInt(
        "Height",
        gui.frame.height,
        { 5, GuiImpl::MaxHeight });
    QObject::connect(
        &gui.settings,
        &SettingsPane::integerChanged,

        [&gui, setting_id_width, setting_id_height] (int id, int value) {
            auto width = id == setting_id_width ? value : gui.frame.width;
            auto height = id == setting_id_height ? value : gui.frame.height;
            emit gui.reset_signal(width, height);
        });

    tool_bar.setToolButtonStyle(Qt::ToolButtonTextOnly);
    tool_bar.setContextMenuPolicy(Qt::PreventContextMenu);
    main_window.addToolBar(Qt::TopToolBarArea, &tool_bar);

    QObject::connect(&gui, &Gui::ready, [this] () {
        main_window.show();
    });
}

void Gui::reset(void)
{
    frame.reset();
    resizeEvent();

    static auto first_run = true;
    if (first_run)
    {
        emit ready();
        first_run = false;
    }
}

void Gui::resizeEvent(void)
{
    // trigger resize event in Frame
    auto e = QResizeEvent { frame.size(), QSize(0, 0) };
    frame.resizeEvent(&e);
}
