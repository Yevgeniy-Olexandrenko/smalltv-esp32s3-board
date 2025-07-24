#include "Settings.h"
#include "drivers/Storage.h"
#include "firmware/defines.h"

bool Settings::m_initData = false;
bool Settings::m_initSets = false;

GyverDBFile Settings::m_keys;
GyverDBFile Settings::m_data;
SettingsESP Settings::m_sets;

core::WebDAV Settings::m_dav;

void Settings::initData()
{
    if (!m_initData)
    {
        auto flashFS = &driver::storage.getFlashFS();
        m_keys.setFS(flashFS, "/settings_keys.db");
        m_data.setFS(flashFS, "/settings_data.db");
        m_keys.begin();
        m_data.begin();
        m_initData = true;
    }
}

void Settings::initSets()
{
    if (!m_initSets)
    {
        m_sets.begin(true, getHostName().c_str());
        m_sets.fs.setFS(driver::storage.getFlashFS());
        m_sets.attachDB(&m_data);
        m_initSets = true;

        m_dav.begin(m_sets.server);
        auto quotaCb = [](fs::FS& fs, uint64_t& available, uint64_t& used)
        {
            auto& fatFS = static_cast<driver::details::FatFS&>(fs);
            available = fatFS.totalBytes() - fatFS.usedBytes();
            used = fatFS.usedBytes();
            log_i(
                "quota callback: %s %llu / %llu",
                fatFS.mountPoint(), used, used + available);
        };
        m_dav.addFS(
            driver::storage.getFlashFS(),
            "Embedded Flash", quotaCb);
        m_dav.addFS(
            driver::storage.getSDCardFS(),
            "External SD Card", quotaCb);
    }
}

GyverDBFile& Settings::data()
{
    initData();
    return m_data;
}

GyverDBFile& Settings::keys()
{
    initData();
    return m_keys;
}

SettingsESP& Settings::sets()
{
    initSets();
    return m_sets;
}

void Settings::tick()
{
    sets().tick();
    keys().tick();
}

const String Settings::getHostName()
{
    return (WIFI_HOST_NAME + getDeviceID());
}

const String Settings::getDeviceID()
{
    String mac = WiFi.macAddress();
    return mac.substring(12, 14) + mac.substring(15);
}

const String Settings::getUserAgent()
{
    return (getHostName() + ("/" PROJECT_VERSION " (contact: " PROJECT_AUTHOR ")"));
}

const String Settings::Provider::led(Led led) const
{
    switch(led)
    {
        case Led::R: return "🔴";
        case Led::Y: return "🟡";
        case Led::G: return "🟢";
    }
    return "";
}
