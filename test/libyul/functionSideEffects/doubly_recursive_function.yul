{
    function a() { b() }
    function b() { a() }
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
