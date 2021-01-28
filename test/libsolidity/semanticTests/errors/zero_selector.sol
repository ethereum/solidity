error overdiffusingness(bytes,uint256,uint256,uint256,uint256);
contract C {
    function f() public pure {
        revert(overdiffusingness("",1,2,3,4));
    }
}
// ====
// compileViaYul: also
// ----
// f() -> FAILURE, hex"00000000", 0xa0, 1, 2, 3, 4, 0
