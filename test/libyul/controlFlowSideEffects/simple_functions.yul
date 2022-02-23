{
    function a() {}
    function f() { g() }
    function g() { revert(0, 0) }
    function h() { stop() }
    function i() { h() }
    function j() { h() g() }
    function k() { g() h() }
}
// ----
// a: can continue
// f: can revert
// g: can revert
// h: can terminate
// i: can terminate
// j: can terminate
// k: can revert
