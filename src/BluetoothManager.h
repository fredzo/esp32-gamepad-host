
#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

/* Functions */

int bluetoothManagerInit(int maxConnections);
void bluetoothManagerRun();
void bluetoothManagerSendOutputReport(Gamepad* gamepad);
void maybeRumble();

#endif
