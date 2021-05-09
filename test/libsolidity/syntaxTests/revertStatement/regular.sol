error E();
contract C {
    function f() public pure {
        revert E();
    }
}
// ----
