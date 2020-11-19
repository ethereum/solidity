contract C {
    function f() public pure {
        uint[] memory x;
        uint y;
        assembly {
            y := x
        }
        assert(y != 0);
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
