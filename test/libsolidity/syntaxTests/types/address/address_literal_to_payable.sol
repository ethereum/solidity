contract C {
    function f() public pure {
        address payable a = payable(address(0x00000000219ab540356cBB839Cbe05303d7705Fa));
        address payable b = payable(0x00000000219ab540356cBB839Cbe05303d7705Fa);
        a = b;
        b = a;
    }
}
// ----
