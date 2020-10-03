#define PernamentPush(size) StoragePush(&os->pernamentStorage, size)
#define TemporaryPush(size) StoragePush(&os->temporaryStorage, size)

internal void *
StoragePush(memory_storage *storage, u64 size)
{
    u64 sizeToAllocate = storage->allocatedSize + size;
    if(sizeToAllocate > storage->maxSize)
    {
        LogError("Reached max allocated memory size!");
        LogError("MaxMemorySize: %llu You wanted to allocate this much: %llu", 
                 storage->maxSize, sizeToAllocate);
        Assert(0);
    }
    u8 *result = (u8 *)storage->memory;

    result += storage->allocatedSize;
    storage->allocatedSize += size;

    if(storage->allocatedSize > storage->highestAllocatedSize)
    {
        storage->highestAllocatedSize = storage->allocatedSize;
    }

    return (void *)result;
}

internal void *
StoragePushZero(memory_storage *storage, u64 size)
{
    void *result = StoragePush(storage, size);
    memset(result, 0, size);

    return result;
}

internal void
StoragePop(memory_storage *storage, u64 size)
{
    if(size > storage->allocatedSize)
    {
        storage->allocatedSize = 0;
    }
    else
    {
        storage->allocatedSize -= size;
    }
}

internal void
StorageReset(memory_storage *storage)
{
    storage->allocatedSize = 0;
}

// Set all bytes in storage to zero and reset status values
internal void
StorageZeroTheWholeThing(memory_storage *storage)
{
    memset(storage->memory, 0, storage->maxSize);
    storage->allocatedSize = 0;
}

internal void
StoragePrint(memory_storage *storage)
{
    LogInfo("MaxSize: %llu AllocatedSize: %llu, highestAllocatedSize: %llu",
        storage->maxSize, storage->allocatedSize, storage->highestAllocatedSize);
}