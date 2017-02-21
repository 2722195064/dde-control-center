#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "contentwidget.h"
#include "moduleinterface.h"
#include "translucentframe.h"

#include <QMap>
#include <QVBoxLayout>
#include <QPushButton>

using namespace dcc;
using namespace dcc::widgets;

class Frame;
class SettingsWidget : public ContentWidget, public FrameProxyInterface
{
    Q_OBJECT

public:
    explicit SettingsWidget(Frame *frame);

signals:
    void requestAutohide(const bool autoHide) const;

public slots:
    void contentPopuped(ContentWidget * const w);
    void showModulePage(const QString &moduleName, const QString &pageName);

    void setFrameAutoHide(ModuleInterface * const inter, const bool autoHide);
    void pushWidget(ModuleInterface * const inter, ContentWidget * const w);

private slots:
    void loadModule(ModuleInterface * const module);
    void onModuleInitFinished(ModuleInterface * const inter);
    void ensureModuleVisible(const QString &moduleName);
    void toggleView();
    void refershModuleActivable();
    void resetAllSettings();

private:
    ~SettingsWidget();

private:
    Frame *m_frame;

    QPushButton *m_resetBtn;
    QVBoxLayout *m_settingsLayout;
    QGridLayout *m_navgationLayout;
    TranslucentFrame *m_settingsWidget;
    TranslucentFrame *m_navgationWidget;

    QList<ModuleInterface *> m_moduleInterfaces;
    QMap<ModuleInterface *, bool> m_moduleActivable;
    QMap<ModuleInterface *, QList<ContentWidget *>> m_moduleWidgets;

    QTimer *m_refershModuleActivableTimer;

    int m_moduleLoadDelay;
    bool m_loadFinished;

    QString m_ensureVisibleModule;
    QString m_ensureVisiblePage;
};

#endif // SETTINGSWIDGET_H
