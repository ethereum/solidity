{
    function a() { b() }
    function b() { a() }
}
// ----
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a:
// b:
