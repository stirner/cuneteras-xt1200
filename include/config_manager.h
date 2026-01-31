#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config.h"

extern Config cfg;

void loadConfig();
void saveConfig();
void resetConfig();

#endif
