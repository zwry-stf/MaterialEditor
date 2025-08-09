#include "hooks.h"
#include <cheat/features/features.h>


void hooks::addHooks()
{
	// chams
	registerHookVTable(features::chams::hkDrawArray, "DrawArray", "scenesystem.dll", "CAnimatableSceneObjectDesc", 1);
}