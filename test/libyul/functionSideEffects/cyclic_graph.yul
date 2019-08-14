{
    function a() { b() }
    function b() { c() }
    function c() { b() }
}
// ----
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a: movable, sideEffectFree, sideEffectFreeIfNoMSize
// b: movable, sideEffectFree, sideEffectFreeIfNoMSize
// c: movable, sideEffectFree, sideEffectFreeIfNoMSize
