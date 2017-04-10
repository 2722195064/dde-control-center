#ifndef DATETIME_H
#define DATETIME_H

#include "datesettings.h"
#include "contentwidget.h"

#include "modulewidget.h"
#include "settingsgroup.h"
#include "settingsitem.h"
#include "settingshead.h"
#include "nextpagewidget.h"
#include "switchwidget.h"

#include <QSettings>

#include <types/zoneinfo.h>

using namespace dcc;

namespace dcc {
namespace datetime {

class DatetimeModel;
class TimeZoneChooser;

class Datetime : public ModuleWidget
{
    Q_OBJECT

public:
    explicit Datetime();
    ~Datetime();

    void setModel(const DatetimeModel *model);

signals:
    void editChanged(bool edit);
    void editDatetime();
    void addClick();

    void requestSetNtp(const bool &ntp);
    void requestTimeSettings();
    void requestAddUserTimeZone(const QString &zone);
    void requestRemoveUserTimeZone(const ZoneInfo &zone);
    void requestSetTimeZone(const QString &zone);

    void requestHold();
    void requestUnhold();

public slots:
    void addTimezone(const ZoneInfo &zone);
    void addTimezones(const QList<ZoneInfo> &zones);
    void removeTimezone(const ZoneInfo &zone);
    void updateTimezoneItems();
    void updateSystemTimezone(const QString &timezone);

private slots:
    void onEditClicked(const bool &edit);

private:
    const DatetimeModel *m_model;

    SettingsGroup *m_timeSettingsGroup;
    SwitchWidget *m_ntpSwitch;
    NextPageWidget *m_timePageButton;

    SettingsGroup *m_timezoneGroup;
    SettingsHead *m_headItem;
    QPushButton *m_addTimezoneButton;

    NextPageWidget *m_timezoneItem;

    bool m_addTimeZone;
    TimeZoneChooser *m_dialog;
};
}
}
#endif // DATETIME_H
