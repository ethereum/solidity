{
    function a() {
        if calldataload(0) { g() }
    }
    function b() {
        g()
        if calldataload(0) { }
    }
    function c() {
        if calldataload(0) { }
        g()
    }
    function d() {
        stop()
        if calldataload(0) { g() }
    }
    function e() {
        if calldataload(0) { g() }
        stop()
    }
    function f() {
        g()
        if calldataload(0) { g() }
    }
    function g() { revert(0, 0) }
}
// ----
// a: can revert, can continue
// b: can revert
// c: can revert
// d: can terminate
// e: can terminate, can revert
// f: can revert
// g: can revert
