#pragma once

extern "C"
{
	__declspec(dllexport) int plugin_is_GPL_compatible;

	int
		emacs_module_init(struct emacs_runtime* runtime);
}
