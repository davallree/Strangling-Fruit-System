# Master controller

This code runs on the master ESP32. The master controller receives hand press
events from the wall controllers via ESPNow, and sends commands to the wall
controllers to update their current animation.

The master will automatically register itself with the wall controllers on boot.
This means that the master must be booted after all the walls. If a wall is
rebooted, the master must also reboot.

TODO(zorg): periodically send commands to walls in order to:
1. Reconnect if the wall was rebooted.
2. Fix any missed command from the master to the wall.
