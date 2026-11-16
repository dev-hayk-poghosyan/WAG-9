#pragma once

namespace DriverFaultIds
{

char const* autoBrakingAfterAlertCheckFailure();

char const* autoBrakingKon();

char const* emergencyBrakingForNoGoodReason();

char const* failureToUseAuxBrakesWhenEmergencyBraking();

char const* exceededSpeedLimitImposedBySign();

char const* exceededTrackSpeedLimit();

char const* failureToSignalAtSign();

char const* collisionWithStationaryObstacle();

char const* collisionWithAnimal();

char const* collisionWithHuman();

char const* passedSignallerWithRedFlagOrLantern();

char const* incorrectlySignalledBeforeSignallerWithRedFlag();

char const* incorrectlySignalledBeforeSignallerWithPermissiveOrWarningSignal();

char const* incorrectlySignalledAfterShunterDirectionCommand();

char const* incorrectlySignalledAfterShunterSlowerCommand();

char const* failureToStopAfterShunterStopCommand();

char const* movingTooFastWhileHitching();

char const* startOfMovementWithoutSignal();

char const* stopWhithoutSignalInShuntingMode();

char const* wrongDesignationOfDriversSeat();

char const* helloSignalToOncomingTrainFault();

char const* goodbyeSignalToOncomingTrainFault();

char const* wrongLocomotiveLightsignalBarrier();

char const* signalToStationSignallerFault();

char const* detonatorHit();

char const* stopWithoutSignal();

char const* stopSignalPassed();

char const* raisedPantographFault();

char const* wrongPantographRaisedFault();

char const* masterKeyOn();

char const* masterKeyOff();

char const* forwardDirection();

char const* backwardDirection();

char const* interventAWS();

char const* interventTPWS();

char const* horn();

char const* spad();

} // namespace DriverFaultIds
