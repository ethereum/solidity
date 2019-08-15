{
    function a() { b() }
    function b() { a() }
}
// ----
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a: movable, sideEffectFree, sideEffectFreeIfNoMSize
// b: movable, sideEffectFree, sideEffectFreeIfNoMSize
