{
    function a() { b() }
    function b() { c() }
    function c() { b() }
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
// c:
