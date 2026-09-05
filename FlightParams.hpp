#pragma once
#include <stdint.h>
#include <stddef.h>

#include "confman.hpp"

#define INF_TIME 1'000'000'000
#define TBD 0

#define SECONDS * 1000

/* FIXED means a reflash of the boards is needed upon modification */
#define FIXED static constexpr
/* DYNA means it is possible to change it through UART*/
#define DYNAMIC

struct PressurizationParams {
    FIXED   uint32_t LaunchDelayMs = 12.f;
    DYNAMIC uint32_t HoldDelayMs   = 9.1f;

    DYNAMIC float TargetPressureLox  = 1.f;
    DYNAMIC float TargetPressureFuel = 1.f;

    /* Ramp Rate (Bar per ms) */
    FIXED float RampRate = 50e-3;
    FIXED float RampExitThresholdRatio = 0.98;
    
    FIXED   float MaxCriticalPressure    = 70.f;
    DYNAMIC float MaxLoxNominalPressure  = 1.5f;
    DYNAMIC float MinLoxNominalPressure  = 0.5f;
    DYNAMIC float MaxFuelNominalPressure = 1.5f;
    DYNAMIC float MinFuelNominalPressure = 0.5f;
};
static_assert(sizeof(PressurizationParams) == 28);

struct IgnitionParams {
    DYNAMIC uint32_t PrechillDurationMs = 1000;
    DYNAMIC uint32_t IgniterDurationMs  = 2000;
    DYNAMIC uint32_t DelayMs            = 500;
    DYNAMIC uint32_t RampUpMs           = 125;

    /* Threshold for liftoff (in m.s^-2) */
    FIXED float    LiftoffAccelThreshold  = 2; // m/s2
    FIXED uint32_t LiftoffAccelDurationMs = 500;
};
static_assert(sizeof(IgnitionParams) == 16);

struct BurnParams {
    FIXED   float    PressureIntegralToImpulse = TBD;
    DYNAMIC uint32_t MinDurationMs             = 1 SECONDS;
    DYNAMIC float    Impulse                   = TBD;
    DYNAMIC uint32_t FcMaxDurationMs           = 5 SECONDS;
    DYNAMIC uint32_t EngineMaxDurationMs       = 0.5 SECONDS;
    DYNAMIC uint32_t CutoffDelayMs             = 40;
};
static_assert(sizeof(BurnParams) == 20);

struct AscentParams {
    FIXED uint32_t AscentMaxDurationMs    = 30 SECONDS;
    FIXED float    DescentSpeedThreshold  = - 3;
    FIXED uint32_t DescentSpeedDurationMs = 300;
};
static_assert(sizeof(AscentParams) == 1);

struct DescentPassivationParams {
    FIXED uint32_t DelayMs      = 5 SECONDS;
    FIXED uint32_t DelayNoComMs = 120 SECONDS;
    
    FIXED uint32_t DurationMs          = 10 SECONDS;
    FIXED uint32_t InterludeDurationMs = 10 SECONDS;
};
struct DescentEngineDepressurizeParams {
    FIXED uint32_t DelayMs    = 10 SECONDS;
    FIXED uint32_t DurationMs = 20 SECONDS;
};
struct DescentDPRDepressurizeParams {
    FIXED uint32_t DelayMs          = 45 SECONDS;
    FIXED float    BallValveOpening = 50.f;
    FIXED uint32_t DurationMs       = 20 SECONDS;
    FIXED uint32_t DelayNoComMs     = 120 SECONDS;
};
struct DescentDepressurizeParams {
    FIXED DescentEngineDepressurizeParams Engine {};
    FIXED DescentDPRDepressurizeParams    DPR    {};
};

struct DescentParams {
    FIXED DescentPassivationParams  Passivation  {};
    FIXED DescentDepressurizeParams Depressurize {};

    FIXED uint32_t MaxDurationMs = 60 SECONDS;
};
static_assert(sizeof(DescentParams) == 1);

struct AbortParams {
    FIXED uint32_t PassivateTimerMs    = 15 SECONDS;
    FIXED uint32_t DepressurizeTimerMs = 15 SECONDS;
};
static_assert(sizeof(AbortParams) == 1);

struct FlightParams {
    DYNAMIC PressurizationParams Pressurization;
    DYNAMIC IgnitionParams       Ignition;
    DYNAMIC BurnParams           Burn;

    FIXED AscentParams  Ascent {};
    FIXED DescentParams Descent {};
    FIXED AbortParams   AIF {};
};
static_assert(sizeof(FlightParams) == sizeof(PressurizationParams) + sizeof(IgnitionParams) + sizeof(BurnParams));

enum BoardIds : uint8_t {
    FP_PRC_ETH = 0,
    FP_PRC_LOX = 1,
    FP_ENGINE  = 2
};

using FlightParamsManager = ConfigManager<FlightParams>;

template<typename Dispatch>
using FlightParamsDispatcher = ConfigDispatcher<FlightParams, Dispatch, 3>;

using FlightParamsCANChannel = CANConfigChannel<FlightParams>;

#define PRINT_FLIGHT_PARAMS(params) { \
    printf("config pressurize hold_delay %u\n",                params.Pressurization.HoldDelayMs); \
    printf("config pressurize target_pressure_lox %f\n",       params.Pressurization.TargetPressureLox); \
    printf("config pressurize target_pressure_fuel %f\n",      params.Pressurization.TargetPressureFuel); \
    printf("config pressurize max_lox_nominal_pressure %f\n",  params.Pressurization.MaxLoxNominalPressure); \
    printf("config pressurize min_lox_nominal_pressure %f\n",  params.Pressurization.MinLoxNominalPressure); \
    printf("config pressurize max_fuel_nominal_pressure %f\n", params.Pressurization.MaxFuelNominalPressure); \
    printf("config pressurize min_fuel_nominal_pressure %f\n", params.Pressurization.MinFuelNominalPressure); \
    printf("config ignition prechill_duration %u\n",           params.Ignition.PrechillDurationMs); \
    printf("config ignition igniter_duration %u\n",            params.Ignition.IgniterDurationMs); \
    printf("config ignition delay %u\n",                       params.Ignition.DelayMs); \
    printf("config ignition ramp_up %u\n",                     params.Ignition.RampUpMs); \
    printf("config burn cutoff_delay %u\n",                    params.Burn.CutoffDelayMs); \
    printf("config burn min_duration %u\n",                    params.Burn.MinDurationMs); \
    printf("config burn impulse %f\n",                         params.Burn.Impulse); \
    printf("config burn max_duration fc %u\n",                 params.Burn.FcMaxDurationMs); \
    printf("config burn max_duration engine %u\n",             params.Burn.EngineMaxDurationMs); \
}

#ifdef COMMANDS_GENERATOR

#include <cstdio>

#define app_printf(...) printf(__VA_ARGS__);

#include <iostream>
int main (void) {
    FlightParams params;

    PRINT_FLIGHT_PARAMS(params);

    std::cerr << "Expected CRC : ";

    FlightParamsManager man;
    std::cerr << man.get_buffer_crc() << std::endl;
}

#endif
