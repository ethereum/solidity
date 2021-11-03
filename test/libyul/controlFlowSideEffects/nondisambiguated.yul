{
    function a() {
        {
            function b() { if calldataload(0) { return(0, 0) } }
            b()
        }
        {
            function b() { revert(0, 0) }
            b()
        }
    }
    {
        function b() {
            leave
            revert(0, 0)
        }
    }
}
// ----
// a: can terminate, can revert
// b: can terminate, can continue
// b: can revert
// b: can continue
