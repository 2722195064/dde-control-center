#include "settingswidget.h"
#include "frame.h"
#include "moduleinitthread.h"
#include "modulewidget.h"

#include "accounts/accountsmodule.h"
#include "bluetooth/bluetoothmodule.h"
#include "datetime/datetimemodule.h"
#include "defapp/defaultappsmodule.h"
#include "keyboard/keyboardmodule.h"
#include "power/powermodule.h"
#include "sound/soundmodule.h"
#include "update/updatemodule.h"
#include "mouse/mousemodule.h"
#include "wacom/wacomemodule.h"
#include "display/displaymodule.h"
#include "personalization/personalizationmodule.h"
#include "systeminfo/systeminfomodule.h"
#include "network/networkmodule.h"

#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScroller>

using namespace dcc::datetime;
using namespace dcc::keyboard;
using namespace dcc::systeminfo;
using namespace dcc::update;

SettingsWidget::SettingsWidget(Frame *frame)
    : ContentWidget(frame),

      m_frame(frame),

      m_resetBtn(new QPushButton),
      m_settingsLayout(new QVBoxLayout),
      m_settingsWidget(new TranslucentFrame),

      m_refershModuleActivableTimer(new QTimer(this)),

      m_moduleLoadDelay(0),
      m_loadFinished(false)
{
    // NOTE: 由于控件的统一风格，ContentWidget 这里有个 spacing item，
    // 但是首页又有神奇的需求不加个这 spacing，所以在这里去掉它
    // sbw
    delete layout()->takeAt(3);

    m_resetBtn->setText(tr("Reset all settings"));
    m_navgationBtn->setVisible(true);

    m_settingsLayout->setSpacing(30);
    m_settingsLayout->setMargin(0);
    m_settingsLayout->addSpacing(10);
//    m_settingsLayout->addWidget(m_resetBtn);
    m_settingsLayout->addSpacing(20);

    loadModule(new accounts::AccountsModule(this));
    loadModule(new display::DisplayModule(this));
    loadModule(new defapp::DefaultAppsModule(this));
    loadModule(new personalization::PersonalizationModule(this));
    loadModule(new network::NetworkModule(this));
    loadModule(new bluetooth::BluetoothModule(this));
    loadModule(new sound::SoundModule(this));
    loadModule(new DatetimeModule(this));
    loadModule(new power::PowerModule(this));
    loadModule(new mouse::MouseModule(this));
    loadModule(new KeyboardModule(this));
    loadModule(new wacom::WacomModule(this));
#ifndef DISABLE_SYS_UPDATE
    loadModule(new UpdateModule(this));
#endif
    loadModule(new SystemInfoModule(this));

    m_settingsWidget->setLayout(m_settingsLayout);

    QVBoxLayout *wrapLayout = new QVBoxLayout;
    wrapLayout->addWidget(m_settingsWidget);
    wrapLayout->setMargin(0);

    QWidget *wrapWidget = new TranslucentFrame;
    wrapWidget->setLayout(wrapLayout);

    m_navgationLayout = new QGridLayout;
    m_navgationWidget = new TranslucentFrame;
    m_navgationWidget->setLayout(m_navgationLayout);
    m_navgationWidget->setParent(wrapWidget);

    setContent(wrapWidget);
    setTitle(tr("All Settings"));

    m_refershModuleActivableTimer->setSingleShot(true);
    m_refershModuleActivableTimer->setInterval(500);

    connect(m_navgationBtn, &QPushButton::clicked, this, &SettingsWidget::toggleView);
    connect(m_resetBtn, &QPushButton::clicked, this, &SettingsWidget::resetAllSettings);
    connect(m_contentArea->verticalScrollBar(), &QScrollBar::valueChanged, m_refershModuleActivableTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_refershModuleActivableTimer, &QTimer::timeout, this, &SettingsWidget::refershModuleActivable);
}

void SettingsWidget::contentPopuped(ContentWidget *const w)
{
    QMap<ModuleInterface *, QList<ContentWidget *>>::iterator it =
        std::find_if(m_moduleWidgets.begin(), m_moduleWidgets.end(),
                     [=](const QList<ContentWidget *> &list) {
                         return !list.isEmpty() && list.back() == w;
                     });

    Q_ASSERT(it != m_moduleWidgets.end());

    it.key()->contentPopped(w);
    it.value().pop_back();
}

void SettingsWidget::setFrameAutoHide(ModuleInterface *const inter, const bool autoHide)
{
    Q_ASSERT(m_moduleInterfaces.contains(inter));

    qDebug() << "setFrameAutoHide: " << inter << inter->name() << autoHide;

    emit requestAutohide(autoHide);
}

void SettingsWidget::pushWidget(ModuleInterface *const inter, ContentWidget *const w)
{
    Q_ASSERT(!m_moduleWidgets[inter].contains(w));

    m_moduleWidgets[inter].append(w);
    m_frame->pushWidget(w);
}

void SettingsWidget::loadModule(ModuleInterface *const module)
{
    Q_ASSERT(!m_moduleInterfaces.contains(module));
    Q_ASSERT(!m_moduleWidgets.contains(module));

    m_moduleInterfaces.append(module);
    m_moduleWidgets.insert(module, QList<ContentWidget *>());

    ModuleInitThread *thrd = new ModuleInitThread(module, this);
    connect(thrd, &ModuleInitThread::moduleInitFinished, this, &SettingsWidget::onModuleInitFinished, Qt::QueuedConnection);
    connect(thrd, &ModuleInitThread::finished, [=] { thrd->exit(); thrd->deleteLater(); });
    QTimer::singleShot(m_moduleLoadDelay, thrd, [=] { thrd->start(QThread::LowestPriority); });

    m_moduleLoadDelay += 50;
    m_moduleLoadDelay %= 340;
}

void SettingsWidget::onModuleInitFinished(ModuleInterface *const module)
{
    Q_ASSERT(m_moduleInterfaces.contains(module));

    // get right position to insert
    int index = 0;
    for (int i(0); i != m_moduleInterfaces.size(); ++i)
    {
        if (m_moduleInterfaces[i] == module)
            break;

        if (m_moduleActivable.contains(m_moduleInterfaces[i]))
            ++index;
    }

    m_moduleActivable[module] = false;
    m_settingsLayout->insertWidget(index + 1, module->moduleWidget());

    // load all modules finished
    if (m_moduleActivable.size() == m_moduleInterfaces.size())
    {
        m_loadFinished = true;
        m_refershModuleActivableTimer->start();

        // scroll to dest widget
        if (!m_ensureVisibleModule.isEmpty())
            QTimer::singleShot(10, this, [=] { showModulePage(m_ensureVisibleModule, m_ensureVisiblePage); });
    }
}

void SettingsWidget::ensureModuleVisible(const QString &moduleName)
{
    ModuleInterface *inter = nullptr;
    for (auto it : m_moduleInterfaces)
    {
        if (it->name() == moduleName)
        {
            inter = it;
            break;
        }
    }

    if (inter)
        scrollToWidget(inter->moduleWidget());
}

void SettingsWidget::toggleView()
{
    m_settingsWidget->setVisible(!m_settingsWidget->isVisible());
    m_navgationWidget->setVisible(!m_navgationWidget->isVisible());
}

void SettingsWidget::showModulePage(const QString &moduleName, const QString &pageName)
{
    m_ensureVisibleModule = moduleName;
    m_ensureVisiblePage = pageName;

    if (!m_loadFinished)
        return;

    if (pageName.isEmpty())
        ensureModuleVisible(moduleName);
}

void SettingsWidget::refershModuleActivable()
{
    stopScroll();

    QScroller *scroller = QScroller::scroller(m_contentArea);
    if (scroller->state() != QScroller::Inactive) {
        m_refershModuleActivableTimer->start();
        return;
    }

    const QRect containerRect = QRect(QPoint(), m_contentArea->size());

    for (ModuleInterface *module : m_moduleInterfaces)
    {
        if (!m_moduleActivable.contains(module))
            continue;

        const QWidget *w = module->moduleWidget();
        Q_ASSERT(w);
        const QRect wRect(w->mapTo(m_contentArea, QPoint()), w->size());
        const bool activable = containerRect.intersects(wRect);

        if (m_moduleActivable[module] == activable)
            continue;

        m_moduleActivable[module] = activable;
        if (activable)
            module->moduleActive();
        else
            module->moduleDeactive();
    }
}

void SettingsWidget::resetAllSettings()
{
    for (auto *inter : m_moduleInterfaces)
        inter->reset();
}

SettingsWidget::~SettingsWidget()
{
    for (auto v : m_moduleWidgets)
        qDeleteAll(v);
    qDeleteAll(m_moduleInterfaces);

    m_settingsWidget->deleteLater();
    m_navgationWidget->deleteLater();
}
