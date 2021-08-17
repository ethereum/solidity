contract C {
    function f() public pure returns (bytes memory) {
        return abi.encodePacked(
            1.23,
            fixed64x4(-1.234)
        );
    }
    function g() public pure returns (bytes memory) {
        return abi.encode(
            1.23,
            fixed64x4(-1.234)
        );
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
// g() ->
