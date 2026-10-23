contract C {
    event E();
}

contract Test is C {
    event E(uint256, uint256);
    function f() public {
        emit C.E();
        emit E(1,2);
    }
}
// ====
// compileViaYul: also
// targetContract: Test
// ----
// f() ->
// ~ emit E()
// ~ emit E(uint256,uint256): 0x01, 0x02
