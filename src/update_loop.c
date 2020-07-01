#include "shared.h"
#include "signal.h"

void Initialize(AppMemory *memory)
{
    raise(SIGABRT);
}
void Update(AppMemory *memory)
{
    raise(SIGABRT);
}
void HotReload(AppMemory *memory)
{

}