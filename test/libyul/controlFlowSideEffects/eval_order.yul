{
    function a() -> x {
        revert(0, 0)
    }
    function b() -> x {
        return(0, 0)
    }
    function c() {
        sstore(a(), b())
    }
    function d() {
        sstore(b(), a())
    }
}
// ----
// a: can revert
// b: can terminate
// c: can terminate
// d: can revert
