{
    function a() { b() }
    function b() { a() }
}
// ----
// : movable, movableIfStateInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
