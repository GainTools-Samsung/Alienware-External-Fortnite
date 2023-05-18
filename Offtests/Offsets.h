#pragma once
#include <Windows.h>

#define OFFSET_UWORLD 0xEF104C0  

namespace Offsets {


	uintptr_t GameInstance = 0x1B8;
	uintptr_t LocalPlayers = 0x38;
	uintptr_t PlayerController = 0x30;
	uintptr_t LocalPawn = 0x330;
	uintptr_t PlayerState = 0x2a8;
	uintptr_t RootComponent = 0x190;
	uintptr_t Persistentlevel = 0x30;
	uintptr_t TeamIndex = 0x10B0;
	uintptr_t AcotrCount = 0xA0;
	uintptr_t AAcotrs = 0x98;
	uintptr_t Mesh = 0x310;
	uintptr_t RelativeLocation = 0x128;
	//uintptr_t DisplayName = 0x1230;
	/*uintptr_t CurrentWeapon = 0x8f0;
	uintptr_t bAlreadySearched = 0x1022;
	uintptr_t WeaponData = 0x3f0;*/
	uintptr_t CurrentActor = 0x8;
	//uintptr_t PrimaryPickupItemEntry = 0x328;
	//uintptr_t ItemDefinition = 0x18;
	uintptr_t Velocity = 0x170;
	uintptr_t ComponentToWorld = 0x240;//0x188
	uintptr_t BoneArray = 0x5F8;
	uintptr_t bIsDBNO = 0x832;

}



namespace W2S {
	uintptr_t chain69 = 0xA8;
	uintptr_t chain699 = 0x7E8;
	uintptr_t chain = 0x70;
	uintptr_t chain1 = 0x98;
	uintptr_t chain2 = 0x140;
	uintptr_t vDelta = 0x10;
	uintptr_t zoom = 0x580;
}