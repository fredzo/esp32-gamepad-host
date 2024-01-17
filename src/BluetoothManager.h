
#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

/* Functions */

int bluetoothManagerInit(int maxConnections);
void bluetoothManagerRun();
void bluetoothManagerSendReport(Gamepad* gamepad, uint16_t* cid);

#endif
