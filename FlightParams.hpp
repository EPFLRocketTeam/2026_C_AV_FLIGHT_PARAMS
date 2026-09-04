
#include <stdint.h>
#include <stddef.h>

#include "confman.hpp"

#define INF_TIME 1'000'000'000
#define SECONDS * 1000
/* FIXED means a reflash of the boards is needed upon modification */
#define FIXED static constexpr

struct PressurizationParams {
    uint32_t HoldDelayMs      = INF_TIME; // N/A
    float LoxSetPressure      = 48.f;
    float FuelSetPressure     = 46.f;
    uint32_t RampUpDurationMs = 10 SECONDS; // N/A
    float MaxCriticalPressure = 65.f;
    float MaxNominalPressure  = 50.f;
    float MinNominalPressure  = 45.f;
};
static_assert(sizeof(PressurizationParams) == 28);

struct IgnitionParams {
    uint32_t PrechillDurationMs     = 200;
    uint32_t IgniterDurationMs      = 2700;
    uint32_t IgnitionDelayMs        = 130;
    uint32_t IgnitionRampUpMs       = 125;

    FIXED float    LiftoffAccelThreshold  = 2; // m/s2
    FIXED uint32_t LiftoffAccelDurationMs = 500;
};
static_assert(sizeof(IgnitionParams) == 16);

struct BurnParams {
    uint32_t CutoffDelayMs     = 40;
    uint32_t BurnMinDurationMs = 2000;
    float    BurnImpulse       = 0; // TBD

    uint32_t FcBurnMaxDurationMs     = 7 SECONDS;
    uint32_t EngineBurnMaxDurationMs = 4 SECONDS;
};
static_assert(sizeof(BurnParams) == 20);

struct DescentParams {
    FIXED uint32_t PassivationDelayMs       = 5 SECONDS;
    FIXED uint32_t DprDepressurizeDelayMs   = 45 SECONDS;
    FIXED uint32_t PassivationDurationMs    = 10 SECONDS;
    FIXED uint32_t InterludeDurationMs      = 10 SECONDS;
    FIXED uint32_t PrcDepressurizeDelayMs   = 10 SECONDS;
    FIXED uint32_t DprPassivationDurationMs = 300 SECONDS;
    FIXED uint32_t PassivationDelayNoComMs  = 120 SECONDS;
    FIXED uint32_t DepressurizeDelayNoComMs = 120 SECONDS;
    FIXED uint32_t DescentMaxDuration       = 60 SECONDS;
};
static_assert(sizeof(DescentParams) == 1);

struct AbortParams {
    FIXED uint32_t PassivateTimerMs    = 15 SECONDS;
    FIXED uint32_t DepressurizeTimerMs = 15 SECONDS;
};
static_assert(sizeof(AbortParams) == 1);

struct FlightParams {
    PressurizationParams Pressurization;
    IgnitionParams       Ignition;
    BurnParams           Burn;

    FIXED DescentParams Descent;
    FIXED AbortParams   AIF;
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
