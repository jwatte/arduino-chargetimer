#include "Settings.h"
#include <WProgram.h>
#include <avr/eeprom.h>


SettingsStore::SettingsStore(void *ptr, unsigned int sz, unsigned int offset) :
    ptr_(ptr),
    sz_(sz),
    offset_(offset)
{
}

bool SettingsStore::load()
{
    eeprom_read_block(ptr_, (char const *)offset_, sz_);
    unsigned int cs = cksum();
    unsigned int ck = eeprom_read_word((uint16_t const *)(offset_ + sz_));
    last_ = (cs == ck);
    if (!last_)
    {
        /*
        Serial.print("cksum "); Serial.print(cs, HEX); Serial.print(" eeprom "); Serial.println(ck, HEX);
        */
        memset(ptr_, 0, sz_);
    }
    return last_;
}

bool SettingsStore::lastOk()
{
    return last_;
}

void SettingsStore::save()
{
    eeprom_write_block(ptr_, (char *)offset_, sz_);
    unsigned int cs = cksum();
    eeprom_write_word((uint16_t *)(offset_ + sz_), cs);
}

unsigned int SettingsStore::cksum()
{
    unsigned int cs = 1;
    for (unsigned char *p = (unsigned char *)ptr_, 
        *e = (unsigned char *)ptr_ + sz_;
        p != e; ++p)
    {
        cs = (cs + *p) * 73 + 109;
    }
    return cs;
}

void SettingsStore::nukeStore()
{
    memset(ptr_, 0, sz_);
    eeprom_write_block(ptr_, (char *)offset_, sz_);
    eeprom_write_word((uint16_t *)(offset_ + sz_), 0);
}
