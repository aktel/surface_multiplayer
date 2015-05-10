//----------------------------------------------------------
//
// Vice city Multiplayer
// Copyright 2014-2015
//
// File Author(s): Jak ozdoba na torcie // adaptation
//					kyeman	// base
//                 	jenksta	// base
// License: See VC:MP License and in root directory
//
//-----------------------------------------------------------

#ifndef VC_VICECITY_HEADER
#define VC_VICECITY_HEADER

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <const.h>
#include <gamedata.h>

#define _pad(x,y) byte x[y]

//-----------------------------------------------------------

#define MAX_PLAYERS             50
#define MAX_VEHICLES    200

//-----------------------------------------------------------

typedef struct _RGBA {
        unsigned char r,g,b,a;
} RGBA, *PRGBA;

#ifndef DYN_VC_ADDRESS

#define CONST_VC_ADDRESSEX( findname, v1address, accessname ) static const dword accessname = v1address; static CConstAddress( findname, v1address ) __info_##accessname; inline dword _##accessname() { __info_##accessname.Debug(); return accessname; }
#define CONST_VC_ADDRESS( v1address, accessname ) CONST_VC_ADDRESS( "##accessname##", v1address, accessname )

#endif

struct Vector3
{
        float X;
        float Y;
        float Z;

        Vector3()
        {
                X = Y = Z = 0;
        }

        Vector3(float fX, float fY, float fZ)
        {
                X = fX; Y = fY; Z = fZ;
        }

        bool IsEmpty() const
        {
                return (X == 0 && Y == 0 && Z == 0);
        }

        float Length() const
        {
                return sqrt((X * X) + (Y * Y) + (Z * Z));
                return 0;
        }

        Vector3 operator+ (const Vector3& vecRight) const
        {
                return Vector3(X + vecRight.X, Y + vecRight.Y, Z + vecRight.Z);
        }

        Vector3 operator+ (float fRight) const
        {
                return Vector3(X + fRight, Y + fRight, Z + fRight);
        }

        Vector3 operator- (const Vector3& vecRight) const
        {
                return Vector3(X - vecRight.X, Y - vecRight.Y, Z - vecRight.Z);
        }

        Vector3 operator- (float fRight) const
        {
                return Vector3(X - fRight, Y - fRight, Z - fRight);
        }

        Vector3 operator* (const Vector3& vecRight) const
        {
                return Vector3(X * vecRight.X, Y * vecRight.Y, Z * vecRight.Z);
        }

        Vector3 operator* (float fRight) const
        {
                return Vector3(X * fRight, Y * fRight, Z * fRight);
        }

        Vector3 operator/ (const Vector3& vecRight) const
        {
                return Vector3(X / vecRight.X, Y / vecRight.Y, Z / vecRight.Z);
        }

        Vector3 operator/ (float fRight) const
        {
                return Vector3(X / fRight, Y / fRight, Z / fRight);
        }

        Vector3 operator - () const
        {
                return Vector3(-X, -Y, -Z);
        }

        void operator += (float fRight)
        {
                X += fRight;
                Y += fRight;
                Z += fRight;
        }

        void operator -= (float fRight)
        {
                X -= fRight;
                Y -= fRight;
                Z -= fRight;
        }

        void operator *= (float fRight)
        {
                X *= fRight;
                Y *= fRight;
                Z *= fRight;
        }

        void operator /= (float fRight)
        {
                X /= fRight;
                Y /= fRight;
                Z /= fRight;
        }
};

//-----------------------------------------------------------
// CEntity

typedef struct ENTITY_TYPE {
        dword     func_table;         // 00-04
        PLACEABLE placeable;          // 04-4C
        dword *   pRWObject;          // 4C-50
        byte      nControlFlags;      // 50-51
        byte      nControlFlags2;     // 51-52
        byte      byteUnkFlags1;      // 52-53
        byte      byteUnkFlags2;      // 53-54
        _pad(__pad1, 0x8);            // 54-5C
        WORD      wModelIndex;        // 5C-5E
        byte      byteBuildingIsland; // 5E-5F
        byte      byteInterior;       // 5F-60
        _pad(__pad3, 0x4);            // 60-64
} CVCBaseEntity;

typedef struct PHYSICAL_TYPE {
        ENTITY_TYPE entity;          // 000-064
        _pad(__pad0a, 0xC);          // 064-070
        Vector3      vecMoveSpeed;    // 070-07C
        Vector3      vecTurnSpeed;    // 07C-088
        _pad(__pad1a, 0x30);         // 088-0B8
        float       fMass;               // 0B8-0BC
        float       fTurnMass;       // 0BC-0C0
        _pad(__pad2a, 0x10);         // 0C0-0D0
        Vector3      vecCenterOfMass; // 0D0-0DC
        _pad(__pad3a, 0x3E);         // 0DC-11A
        byte        byteSunkFlags;   // 11A-11B
        byte        byteLockedFlags; // 11B-11C
        _pad(__pad4a, 0x4);          // 11C-120
} CVCPsyhical;

//-----------------------------------------------------------

//-----------------------------------------------------------
// Weapon State Enumeration

enum eWeaponState
{
        WS_NONE,
        WS_FIRING, // seems to be firing
        WS_RELOADING // seems to be reloading
};

//-----------------------------------------------------------

typedef struct _WEAPON_SLOT {
        dword dwType;        // 00-04
        dword dwState;       // 04-08
        dword dwAmmoInClip;  // 08-0C
        dword dwAmmo;        // 0C-10
        _pad(__pad0a, 0x8);  // 10-18
} WEAPON_SLOT;

//-----------------------------------------------------------
// CPed

typedef struct PED_TYPE {
        PHYSICAL_TYPE physical;        // 000-120
        _pad(__pad0b, 0x2C);           // 120-14C
        byte          byteShootFlags;  // 14C-14D
        byte          byteJumpFlags;   // 14D-14E
        _pad(__pad1b, 0x22);           // 14E-170
        dword *       pTargetEntity;   // 170-174
        _pad(__pad2b, 0xD0);           // 174-244
        byte          byteAction;      // 244-245
        _pad(__pad3b, 0x10F);          // 245-354
        float         fHealth;         // 354-358
        float         fArmour;         // 358-35C
        _pad(__pad4b, 0x18);           // 35C-374
        float         fRotation1;      // 374-378
        float         fRotation2;      // 378-37C
        _pad(__pad5b, 0x28);           // 37C-3A4
        // (3A0 seems to be some vehicle objective)
        dword *       pLastVehicle;    // 3A4-3A8
        dword *       pVehicle;        // 3A8-3AC
        byte          byteIsInVehicle; // 3AC-3AD
        _pad(__pad6b, 0x27);           // 3AD-3D4
        byte          bytePedType;     // 3D4-3D5
        _pad(__pad7b, 0x33);           // 3D5-408
        WEAPON_SLOT   weaponSlots[10]; // 408-4F8
        _pad(__pad8b, 0x8);            // 4F8-500
        dword         dwCurWeaponAmmo; // 500-504
        dword         dwCurrentWeapon; // 504-508
        _pad(__pad9b, 0x90);           // 508-598
        dword         dwWeaponUsed;    // 598-59C
        dword *       pDamageEntity;   // 59C-5A0
        _pad(__pad10b, 0x6C);          // 5A0-60C
        byte              byteCurWepSlot;  // 60C-60D
        // 6D8 = sizeof(CPlayerPed)?
} CPedEntity;

//-----------------------------------------------------------
// CVehicleHandling

struct VEHICLE_HANDLING_TYPE
{
        _pad(__pad0e, 0xCC); // 00-CC
        dword dwFlags;       // CC-D0
        _pad(__pad1e, 0xC);  // D0-DC
};

//-----------------------------------------------------------
// CVehicle

typedef struct VEHICLE_TYPE {
        PHYSICAL_TYPE           physical;           // 000-120
        VEHICLE_HANDLING_TYPE * pHandling;          // 120-124
        _pad(__pad0b, 0x7C);                        // 124-1A0
        byte                    byteColors[4];      // 1A0-1A4
        _pad(__pad1b, 0x4);                         // 1A4-1A8
        PED_TYPE *              pDriver;            // 1A8-1AC
        PED_TYPE *              pPassengers[8];     // 1AC-1CC
        byte                    bytePassengerCount; // 1CC-1CD
        _pad(__pad3b, 0x3);                         // 1CD-1D0
        byte                    byteMaxPassengers;  // 1D0-1D1
        _pad(__pad4b, 0x17);                        // 1D1-1E8
        float                   fSteerAngle1;       // 1E8-1EC
        float                   fSteerAngle2;       // 1EC-1F0
        float                   fAcceleratorPedal;  // 1F0-1F4
        float                   fBrakePedal;        // 1F4-1F8
        _pad(__pad5b, 0xC);                         // 1F8-204
        float                   fHealth;            // 204-208
        _pad(__pad6b, 0x28);                        // 208-230
        dword                   dwDoorsLocked;      // 230-234
        dword                   dwWeaponUsed;       // 234-238
        dword *                 pDamageEntity;      // 238-23C
        dword                   nRadio;             // 23C-240
        byte                    byteHorn;           // 240-241
        dword                   dwUnk1;             // 241-245
        byte                    byteSiren;          // 245-246
        _pad(__pad7b, 0x5A);                        // 246-2A0
} CVehicleEntity;

//-----------------------------------------------------------
// CAutomobile

typedef struct AUTOMOBILE_TYPE
{
        VEHICLE_TYPE vehicle;                 // 000-2A0
        _pad(__pad0c, 0x4);                   // 2A0-2A4
        byte byteEngineStatus;                // 2A4-2A5
        byte byteTireStatus[4];               // 2A5-2A9
        byte byteComponentStatus[6];          // 2A9-2AF
        _pad(__pad1c, 0x1);                   // 2AF-2B0
        dword dwLightStatus;                  // 2B0-2B4
        dword dwPanelStatus;                  // 2B4-2B8
        _pad(__pad2c, 0x249);                 // 2B8-501
        byte byteUnknownFlags;                            // 501-502
        _pad(__pad3c, 0xAE);                  // 502-5B0
        float        fSpecialWeaponRotation1; // 5B0-5B4 (following 2 are rhino turret and firetruck spray)
        float        fSpecialWeaponRotation2; // 5B4-5B8
        _pad(__pad4c, 0x24);                  // 5B8-5DC
} CAutomobileEntity;

//-----------------------------------------------------------

typedef struct _CAMERA_AIM
{
        Vector3 vecA1; // float f1x,f1y,f1z
        Vector3 vecAPos1; // float pos1x,pos1y,pos1z
        Vector3 vecAPos2; // float pos2x,pos2y,pos2z
        Vector3 vecA2; // float f2x,f2y,f2z
} CAMERA_AIM;

//-----------------------------------------------------------
// CCamera

typedef struct _CAMERA_TYPE
{
        _pad(__pad0, 0x190); // 000-190
        byte byteDriveByLeft; // 190-191
        byte byteDriveByRight; // 191-192
        _pad(__pad1, 0x15E); // 192-2F0
        CAMERA_AIM aim;      // 2F0-320
        _pad(__pad2, 0x41C); // 320-73C
        Vector3 vecPosition;  // 73C-748
        Vector3 vecRotation;  // 748-754
        _pad(__pad3, 0x114); // 754-868
        byte byteInFreeMode; // 868-869
        _pad(__pad4, 0xEF);  // 869-958
} CAMERA_TYPE;

//-----------------------------------------------------------
// Weather IDs

#define WEATHER_SUNNY           0
#define WEATHER_CLOUDY          1
#define WEATHER_RAINING         2
#define WEATHER_FOGGY           3
#define WEATHER_EXTRASUNNY      4
#define WEATHER_STORM           5
#define WEATHER_INTERIOR        6

//-----------------------------------------------------------
// Interior IDs

#define INTERIOR_OUTSIDE        0
#define INTERIOR_HOTEL          1
#define INTERIOR_MANSION        2
#define INTERIOR_BANK           3
#define INTERIOR_MALL           4
#define INTERIOR_STRIPCLUB      5
#define INTERIOR_LAWYERS        6
#define INTERIOR_CAFEROBINA     7
#define INTERIOR_CONCERT        8
#define INTERIOR_STUDIO         9
#define INTERIOR_AMMUNATION     10
#define INTERIOR_APPARTMENT     11
#define INTERIOR_POLICEHQ       12
#define INTERIOR_UNKNOWN        12
#define INTERIOR_STADIUM1       14
#define INTERIOR_STADIUM2       15
#define INTERIOR_STADIUM3       16
#define INTERIOR_CLUB           17
#define INTERIOR_PRINTWORKS     18

//-----------------------------------------------------------
// Fade Types

#define FADE_OUT                        0
#define FADE_IN                         1

//-----------------------------------------------------------
// Vehicle Subtypes

#define VEHICLE_SUBTYPE_NONE            0
#define VEHICLE_SUBTYPE_CAR                             1
#define VEHICLE_SUBTYPE_BIKE                    2
#define VEHICLE_SUBTYPE_HELI                    3
#define VEHICLE_SUBTYPE_BOAT                    4
#define VEHICLE_SUBTYPE_PLANE                   5

//-----------------------------------------------------------
// Action Types

#define ACTION_NORMAL 1
#define ACTION_DRIVING_VEHICLE 50
#define ACTION_WASTED 55
#define ACTION_GETTING_IN_VEHICLE 58
#define ACTION_EXITING_VEHICLE 60

//-----------------------------------------------------------
// Weapon Model IDs

#define WEAPON_MODEL_CELLPHONE                  258
#define WEAPON_MODEL_BRASSKNUCKLE               259
#define WEAPON_MODEL_SCREWDRIVER                260
#define WEAPON_MODEL_GOLFCLUB                   261
#define WEAPON_MODEL_NITESTICK                  262
#define WEAPON_MODEL_KNIFECUR                   263
#define WEAPON_MODEL_BASEBALL_BAT               264
#define WEAPON_MODEL_HAMMER                             265
#define WEAPON_MODEL_CLEAVER                    266
#define WEAPON_MODEL_MACHETE                    267
#define WEAPON_MODEL_KATANA                             268
#define WEAPON_MODEL_CHAINSAW                   269
#define WEAPON_MODEL_GRENADE                    270
#define WEAPON_MODEL_TEARGAS                    271
#define WEAPON_MODEL_MOLOTOV                    272
#define WEAPON_MODEL_MISSILE                    273
#define WEAPON_MODEL_COLT45                             274
#define WEAPON_MODEL_PYTHON                             275
#define WEAPON_MODEL_RUGER                              276
#define WEAPON_MODEL_CHROMEGUN                  277
#define WEAPON_MODEL_SHOTGSPA                   278
#define WEAPON_MODEL_BUDDYSHOT                  279
#define WEAPON_MODEL_M4                                 280
#define WEAPON_MODEL_TEC9                               281
#define WEAPON_MODEL_UZI                                282
#define WEAPON_MODEL_INGRAMSL                   283
#define WEAPON_MODEL_MP5LNG                             284
#define WEAPON_MODEL_SNIPER                             285
#define WEAPON_MODEL_LASER                              286
#define WEAPON_MODEL_ROCKETLA                   287
#define WEAPON_MODEL_FLAME                              288
#define WEAPON_MODEL_M60                                289
#define WEAPON_MODEL_MINIGUN                    290
#define WEAPON_MODEL_BOMB                               291
#define WEAPON_MODEL_CAMERA                             292
#define WEAPON_MODEL_FINGERS                    293
#define WEAPON_MODEL_MINIGUN2                   294

//-----------------------------------------------------------
// Weapon IDs

#define WEAPON_UNARMED                                  0
#define WEAPON_BRASSKNUCKLE                             1
#define WEAPON_SCREWDRIVER                              2
#define WEAPON_GOLFCLUB                                 3
#define WEAPON_NITESTICK                                4
#define WEAPON_KNIFECUR                                 5
#define WEAPON_BASEBALL_BAT                             6
#define WEAPON_HAMMER                                   7
#define WEAPON_CLEAVER                                  8
#define WEAPON_MACHETE                                  9
#define WEAPON_KATANA                                   10
#define WEAPON_CHAINSAW                                 11
#define WEAPON_GRENADE                                  12
#define WEAPON_TEARGAS                                  14
#define WEAPON_MOLOTOV                                  15
#define WEAPON_MISSILE                                  16
#define WEAPON_COLT45                                   17
#define WEAPON_PYTHON                                   18
#define WEAPON_CHROMEGUN                                19
#define WEAPON_SHOTGSPA                                 20
#define WEAPON_BUDDYSHOT                                21
#define WEAPON_TEC9                                             22
#define WEAPON_UZI                                              23
#define WEAPON_INGRAMSL                                 24
#define WEAPON_MP5LNG                                   25
#define WEAPON_M4                                               26
#define WEAPON_RUGER                                    27
#define WEAPON_SNIPER                                   28
#define WEAPON_LASER                                    29
#define WEAPON_ROCKETLA                                 30
#define WEAPON_FLAME                                    31
#define WEAPON_M60                                              32
#define WEAPON_MINIGUN                                  33
#define WEAPON_BOMB                                             34
#define WEAPON_HELICANNON                               35
#define WEAPON_CAMERA                                   36
#define WEAPON_COLLISION                                39
#define WEAPON_FALL                                             41
#define WEAPON_DRIVEBY                                  42
#define WEAPON_DROWN                                    43
#define WEAPON_WATER                                    50
#define WEAPON_EXPLOSION                                51

//-----------------------------------------------------------
//-----------------------------------------------------------

#define ENTITY_TYPE_UNKNOWN             0
#define ENTITY_TYPE_PED                 1
#define ENTITY_TYPE_VEHICLE             2

class CVCSurfEntity
{
private:
        ENTITY_TYPE     * m_pEntity;

public:
        ENTITY_TYPE * GetEntity();
        void              SetEntity(ENTITY_TYPE * pEntity);

        void              GetMatrix(MATRIX4X4 * matMatrix);
        void              SetMatrix(MATRIX4X4 matMatrix);
        void          GetPosition(Vector3 * vecPosition);
        void          SetPosition(Vector3 vecPosition);
        void          SetHeading(float fHeading);
        void          Teleport(float fX, float fY, float fZ);

        WORD              GetModelIndex();
        void          SetModelIndex(WORD wModelIndex);

        BOOL          IsOnScreen();

        float         GetDistanceFromCentreOfMassToBaseOfModel();
};

class CVCSurfPhysical : public CVCSurfEntity
{
public:
        PHYSICAL_TYPE * GetPhysical();

        void GetMoveSpeed(Vector3 * vecMoveSpeed);
        void SetMoveSpeed(Vector3 vecMoveSpeed);
        void GetTurnSpeed(Vector3 * vecMoveSpeed);
        void SetTurnSpeed(Vector3 vecMoveSpeed);
};

CONST_VC_ADDRESSEX( "ÑCamera Camera", 0x7E4688, Camera );
CONST_VC_ADDRESSEX( "CCamera::Restore", 0x46BC7D, CCameraResore );
CONST_VC_ADDRESSEX( "CCamera::PutBehindPlayer", 0x46BADE, CCameraPutBehindPlayer );
CONST_VC_ADDRESSEX( "CCamera::SetTargetPoint", 0x46A494, CCameraSetTargetPoint );                    
CONST_VC_ADDRESSEX( "CCamera::SetPositionAndRotation", 0x46BA72, CCamereSetPositionAndRotation );

class CVCCamera
{
private:
        CAMERA_TYPE * m_pCamera;

public:
        CCamera();
        ~CCamera();

        void          SetCamera(CAMERA_TYPE * pCamera);
        CAMERA_TYPE * GetCamera();
        void          SetBehindPlayer();
        void          SetPosition(Vector3 vecPosition);
        void          SetRotation(Vector3 vecRotation);
        void          LookAtPoint(Vector3 vecPoint, int iType);
        void          Restore();
        void          SetInFreeMode(bool bFreeMode);
        bool          IsInFreeMode();
        void          SetDriveByLeft(BYTE byteDriveByLeft);
        BYTE          GetDriveByLeft();
        void          SetDriveByRight(BYTE byteDriveByRight);
        BYTE          GetDriveByRight();
        void          SetAim(CAMERA_AIM * pAim);
        CAMERA_AIM *  GetAim();
};

CONST_VC_ADDRESS( RequestModel 0x40E310 );
CONST_VC_ADDRESS( LoadRequestedModels, 0x40B5F0 );
CONST_VC_ADDRESS( ModelInfo, 0x94DDD8 );
CONST_VC_ADDRESSEX( "CClock::SetTime", 0x487160, CClockSetTime );
CONST_VC_ADDRESS( PlayerCash, 0x94ADC8 );// This is actually CPlayerInfo[0].Cash
CONST_VC_ADDRESS( CreateMarker, 0x4C3C80 );
CONST_VC_ADDRESS( ShowMarker 0x4C3840 );
CONST_VC_ADDRESS( SetMarkerIcon, 0x4C3780 );
CONST_VC_ADDRESS( SetMarkerColor, 0x4C3930 );
CONST_VC_ADDRESS( GameState, 0x9B5F08 );
CONST_VC_ADDRESS( GlobalGravity, 0x68F5F0 );

enum eGameState
{
        GS_START,
        GS_SHOWLOGO,
        GS_LOGOPROCESS,
        GS_SHOWTITLES,
        GS_TITLESPROCESS,
        GS_PREINITGAME,
        GS_SELVIDEOMODE,
        GS_MENUPROCESS,
        GS_LOADGAME,
        GS_SHUTDOWN,
};

#endif // VC_VICECITY_HEADER