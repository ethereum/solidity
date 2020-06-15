{
    function a() { a() }
}
// ----
// : movable, movableIfStateInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
