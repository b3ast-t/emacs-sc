#include "pch.h"
#include "emacsmod.h"


__declspec(dllexport)
int emacs_module_init(emacs_runtime* runtime)
{
    if (runtime->size < sizeof(*runtime))
        return 1;
}