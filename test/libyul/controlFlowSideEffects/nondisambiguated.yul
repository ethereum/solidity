{
    function a() {
        {
            function b() { if calldataloda(0) { return(0, 0) } }
            b()
        }
        {
            function b() { revert(0, 0) }
            b()
        }
    }
    function b() {
        leave
        revert(0, 0)
    }
}
// ----
// a: can revert
// b: can continue
