contract C {
    function f() public pure {
        assembly { mstore(0,0) }
    }
}
// ----
// :C(creation) true
// :C(runtime) false
