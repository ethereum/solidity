pragma abicoder               v2;
contract C {
    function f() public pure returns (bytes memory, bytes memory) {
        return (abi.encode(""), abi.encodePacked( "7?8r"));
    }
}
// ====
// SMTEngine: all
// ----
