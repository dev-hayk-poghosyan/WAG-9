#include "DriverFaultIds.h"

namespace DriverFaultIds
{

char const* autoBrakingAfterAlertCheckFailure()
{
    return "auto_braking_after_alert_check_failure";
}

char const* autoBrakingKon()
{
    return "auto_braking_kon";
}

char const* emergencyBrakingForNoGoodReason()
{
    return "emergency_braking_for_no_good_reason";
}

char const* failureToUseAuxBrakesWhenEmergencyBraking()
{
    return "failure_to_use_aux_brakes_when_emergency_braking";
}

char const* exceededSpeedLimitImposedBySign()
{
    return "exceeded_speed_limit_imposed_by_sign";
}

char const* exceededTrackSpeedLimit()
{
    return "exceeded_track_speed_limit";
}

char const* failureToSignalAtSign()
{
    return "failure_to_signal_at_sign";
}

char const* collisionWithStationaryObstacle()
{
    return "collision_with_stationary_obstacle";
}

char const* collisionWithAnimal()
{
    return "collision_with_animal";
}

char const* collisionWithHuman()
{
    return "collision_with_human";
}

char const* passedSignallerWithRedFlagOrLantern()
{
    return "passed_signaller_with_red_flag";
}

char const* incorrectlySignalledBeforeSignallerWithRedFlag()
{
    return "incorrectly_signalled_before_signaller_with_red_flag";
}

char const* incorrectlySignalledBeforeSignallerWithPermissiveOrWarningSignal()
{
    return "incorrectly_signalled_before_signaller_with_permissive_or_warning_signal";
}

char const* incorrectlySignalledAfterShunterDirectionCommand()
{
    return "incorrectly_signalled_after_shunter_direction_command";
}

char const* incorrectlySignalledAfterShunterSlowerCommand()
{
    return "incorrectly_signalled_after_shunter_slower_command";
}

char const* failureToStopAfterShunterStopCommand()
{
    return "failure_to_stop_after_shunter_stop_command";
}

char const* movingTooFastWhileHitching()
{
    return "moving_too_fast_while_hitching";
}

char const* startOfMovementWithoutSignal()
{
    return "start_of_movement_without_signal";
}

char const* stopWhithoutSignalInShuntingMode()
{
    return "stop_of_movement_whithout_signal_in_shunting_mode";
}

char const* wrongDesignationOfDriversSeat()
{
    return "wrong_designation_of_drivers_seat";
}

char const* helloSignalToOncomingTrainFault()
{
    return "hello_signal_to_oncoming_train_fault";
}

char const* goodbyeSignalToOncomingTrainFault()
{
    return "goodbye_signal_to_oncoming_train_fault";
}

char const* wrongLocomotiveLightsignalBarrier()
{
    return "wrong_locomotive_lightsignal_barrier";
}

char const* signalToStationSignallerFault()
{
    return "signal_to_station_signaller_fault";
}

char const* detonatorHit()
{
    return "detonator_hit";
}

char const* stopWithoutSignal()
{
    return "stop_without_signal";
}

char const* stopSignalPassed()
{
    return "stop_signal_passed";
}

char const* raisedPantographFault()
{
    return "raised_pantograph_fault";
}

char const* wrongPantographRaisedFault()
{
    return "wrong_pantograph_raised_fault";
}

char const* masterKeyOn()
{
    return "master_key_on";
}

char const* masterKeyOff()
{
    return "master_key_off";
}

char const* forwardDirection()
{
    return "forward_direction";
}

char const* backwardDirection()
{
    return "backward_direction";
}

char const* interventAWS()
{
    return "aws_intervention";
}

char const* interventTPWS()
{
    return "tpws_intervention";
}

char const* horn()
{
    return "warning_horn";
}

char const* spad()
{
    return "spad";
}

} // namespace DriverFaultIds
