{
    function a() { b() }
    function b() { c() }
    function c() { b() }
}
// ----
// : movable, movableIfStateInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
// c:
