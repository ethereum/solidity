{
    function a() { pop(gas()) }
    function f() { a() }
    function g() { stop() }
    function h() { invalid() }
}
// ----
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a: sideEffectFree, sideEffectFreeIfNoMSize
// f: sideEffectFree, sideEffectFreeIfNoMSize
// g:
// h:
