
#if !defined(SETTINGS_H)
#define SETTINGS_H

//  SettingsStore stores some data in EEPROM, with a 2 byte 
//  checksum tacked on the end. It uses block transfer, to avoid
//  slow byte transfers (and additional write cycles).
class SettingsStore
{
public:
    SettingsStore(void *ptr, unsigned int sz, unsigned int offset);
    bool load();
    bool lastOk();
    void save();
    void nukeStore();
private:
    unsigned int cksum();
    void *ptr_;
    unsigned int sz_;
    unsigned int offset_;
    bool last_;
};

//  A simple helper template for accessing settings
template<typename T> class Settings : public SettingsStore
{
public:
    Settings() : SettingsStore(&data, sizeof(data), 2) {}
    T *operator->() { return &data; }
private:
    T data;
};

#endif  //  SETTINGS_H
