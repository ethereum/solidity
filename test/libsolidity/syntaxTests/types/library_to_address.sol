library L {
}
contract C {
    function f() public pure returns (address) {
        return address(L);
    }
}
// ----
