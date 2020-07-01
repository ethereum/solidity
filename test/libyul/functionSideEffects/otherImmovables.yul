{
    function a() { pop(gas()) }
    function f() { a() }
    function g() { stop() }
    function h() { invalid() }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: can be removed, can be removed if no msize
// f: can be removed, can be removed if no msize
// g:
// h:
