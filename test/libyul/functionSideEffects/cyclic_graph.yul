{
    function a() { b() }
    function b() { c() }
    function c() { b() }
}
// ----
// : movable, movableIfStateInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
// c:
