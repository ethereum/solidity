{
    function a() { a() }
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
