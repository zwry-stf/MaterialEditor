#include "base.h"
#include "init/vtable.h"
#include "init/signatures.h"
#include "init/hooks.h"
#include "init/functions.h"
#include "init/interfaces.h"
#include "render/render.h"
#include <cheat/features/features.h>


void base::init()
{
	vtable::init();

	signatures::init();

	interfaces::init();

	functions::init();

	features::chams::init();

	render::init();

	hooks::init();

	render::initMouse();
}

void base::destroy()
{
	hooks::destroy();

	render::destroy();
}