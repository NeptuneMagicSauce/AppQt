#include "Gui.hpp"

#include <cmath>
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
    GuiImpl(Gui& gui, const float& ratio);

    static constexpr int MaxWidth = 50;
    static constexpr int MaxHeight = 40;

private:
    Gui& gui;
    const float& ratio;
    EventFilterFirstShow filter;

    QMainWindow main_window;
    QToolBar tool_bar;

    QToolButton* formatAction(QAction* action)
    {
        action->setToolTip(
            action->toolTip() + " (" +
            action->shortcut().toString() + ")");
        auto font = action->font();
        font.setPointSize(16);
        action->setFont(font);
        QToolButton* ret = nullptr;
        for (auto* w: action->associatedWidgets())
        {
            auto* button = dynamic_cast<QToolButton*>(w);
            if (button != nullptr)
            {
                button->setAutoRaise(false);
                ret = button;
            }
        }
        return ret;
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

Gui::Gui(const int& width, const int& height, const float& ratio) :
    frame(width, height, GuiImpl::MaxWidth, GuiImpl::MaxHeight),
    settings(&frame)
{
    Utils::assertSingleton(typeid(*this));
    new GuiImpl(*this, ratio);
    reset();
}

GuiImpl::GuiImpl(Gui& gui, const float& ratio) :
    gui(gui),
    ratio(ratio)
{
    main_window.setCentralWidget(&gui.frame);
    main_window.setWindowTitle("Super Minus");

    filter.callback = [this] () { setInitialWindowSize(); };
    tool_bar.installEventFilter(&filter);
    tool_bar.setFloatable(false);
    tool_bar.setMovable(false);

    auto spacer = [] () {
        auto spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        return spacer;
    };

    tool_bar.addWidget(spacer());

    auto action_reset = tool_bar.addAction(
        Labels::reset,
        [&gui]() {
            emit gui.reset_signal(gui.frame.width, gui.frame.height);
        });
    action_reset->setToolTip("Reset");
    action_reset->setShortcut(QKeySequence::Refresh);
    formatAction(action_reset);

    tool_bar.addWidget(spacer());

    auto addActionEndToolBar = [this, action_reset] (QAction* action) {
        tool_bar.addAction(action);
        auto button = formatAction(action);
        auto spacer_for_centering = new QWidget;
        spacer_for_centering->setFixedSize(button->sizeHint());
        tool_bar.insertWidget(action_reset, spacer_for_centering);
    };

    auto settings_action = gui.settings.action(Labels::settings);
    addActionEndToolBar(settings_action);

    QObject::connect(&gui.frame, &FrameInputEvents::anyActivity,
                     [settings_action] () {
                         settings_action->setChecked(false);
                     });

    gui.settings.integer(
        "Width", "",
        gui.frame.width,
        { 5, GuiImpl::MaxWidth },
        5,
        [this] (QVariant value) {
            emit this->gui.reset_signal(value.toInt(), this->gui.frame.height);});
    gui.settings.integer(
        "Height", "",
        gui.frame.height,
        { 5, GuiImpl::MaxHeight },
        5,
        [this] (QVariant value) {
            emit this->gui.reset_signal(this->gui.frame.width, value.toInt());});
    gui.settings.integer(
        "Mines", "%",
        std::round(ratio * 100),
        { 10, 90 },
        10,
        [this] (QVariant value) {
            emit this->gui.reset_ratio(float(value.toInt()) / 100);
        });
#warning TODO here
    // TODO do not have two signals with strange names ...
    // call setters width height ratio, than reset(void);

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
