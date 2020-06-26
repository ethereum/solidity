{
    function a() { a() }
}
// ----
// : movable, movableIfStateInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
