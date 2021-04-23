contract C {
    function f() public pure returns (fixed) {
        return 1.33;
    }
    function g(fixed f1, fixed f2) public pure returns (fixed, fixed, fixed) {
        return (f2, 0.0000333, f1);
    }
}
// ----
// f() ->
// g(fixed128x80,fixed128x80): 9.871, 88888888.0 ->
