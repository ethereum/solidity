{
    function a() {
        revert(0, 0)
        leave
    }
    function b() {
        leave
        revert(0, 0)
    }
}
// ----
// a: can revert
// b: can continue
