contract C {
    function f() public pure {
        uint[] memory x;
        uint y;
        assembly {
            y := x
        }
        // The value of an uninitialized dynamic array is not zero but rather
        // an address of a location in memory that has the value of zero.
        assert(y != 0);
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
