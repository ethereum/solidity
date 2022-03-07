contract C {
    function f() public pure {
        bytes memory x;
        assembly {
            x := 0
        }
    }
}
// ----
// :C(creation) true
// :C(runtime) false
