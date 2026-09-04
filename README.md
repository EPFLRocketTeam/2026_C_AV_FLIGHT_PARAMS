
# Flight Parameters

This repository contains the default flight parameters for Firehorn II.

## Updating parameters on the fly

One can update parameters of the propulsion system on the fly provided these aren't `FIXED` parameters as defined in the `FlightParams.hpp` file.

If you modify the repo but don't want to reflash the boards, you can obtain the commands to send through UART to the FC in `INIT` mode by running the following command :

```sh
g++ -o build FlightParams.cpp -DCOMMANDS_GENERATOR -I2026_C_AV_CONFMAN/ && ./build
```

This will yield something like

```
config pressurize hold_delay 1000000000
config pressurize lox_set_pressure 48.000000
config pressurize fuel_set_pressure 46.000000
config pressurize ramp_up_duration 10000
config pressurize max_critical_pressure 65.000000
config pressurize max_nominal_pressure 50.000000
config pressurize min_nominal_pressure 45.000000
config ignition prechill_duration 200
config ignition igniter_duration 2700
config ignition delay 130
config ignition ramp_up 125
config burn cutoff_delay 40
config burn min_duration 2000
config burn impulse 0.000000
config burn max_duration fc 7000
config burn max_duration engine 4000
```

that you can then send through UART to set the propulsion behaviour. This should only be used in the context of the VSFT campaign, and for test runs (not the final VSFT). It is recommended to flash the boards in the rest of the campaign.
