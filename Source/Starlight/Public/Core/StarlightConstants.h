#pragma once

/* Trace channel used by portal component to spawn portals */
#define ECC_Portal ECC_GameTraceChannel1
/* Object type of actor copies created by portals with type EPortalType::First */
#define ECC_FirstPortalCopy ECC_GameTraceChannel2
/* Object type of actor copies created by portals with type EPortalType::Second */
#define ECC_SecondPortalCopy ECC_GameTraceChannel3
/* Object type of portal body */
#define ECC_PortalBody ECC_GameTraceChannel4
/* Trace channel used to determine if vision to grabbed object is obstructed */
#define ECC_GrabObstruction ECC_GameTraceChannel5
/* Object type of teleportable objects inside first portal's inner collision box */
#define ECC_WithinFirstPortal ECC_GameTraceChannel6
/* Object type of teleportable objects inside second portal's inner collision box */
#define ECC_WithinSecondPortal ECC_GameTraceChannel7
/* Object type of teleportable objects inside both first and second portals' inner collision boxes */
#define ECC_WithinBothPortals ECC_GameTraceChannel8

const TSet TeleportingCopyTypes = {ECC_FirstPortalCopy, ECC_SecondPortalCopy}; 