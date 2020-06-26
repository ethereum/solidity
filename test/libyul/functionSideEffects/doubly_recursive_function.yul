{
    function a() { b() }
    function b() { a() }
}
// ----
// : movable, movableIfStateInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
