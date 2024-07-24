# Wall controller

This code runs on 4 ESP32s, one per wall on the cube. Wall controllers do not
have any state on the cube as a whole: they just send hand pressed/released
events to the master controller, and update their LED animation via commands
sent over ESPNow.

Wall controllers have multiple LED animations at their disposal.

TODO(zorg): come up with an easy way to add animations (animation registry). For
example, the master could pick which animations to cycle through in the
"ambient" mode. Commands could also have arguments to e.g. use the same
animation with varying levels of "intensity" when walls are touched.

Wall controllers do not know the master's MAC address: the master is expected to
connect to them when the master boots up, at which point the wall will save the
master's MAC address. If a wall is rebooted, the master needs to reboot as well.
