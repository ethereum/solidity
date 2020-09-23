contract C {
    function f() public pure returns (address) {
        return address(2**160 -1);
    }
    function g() public pure returns (address) {
        return address(uint(-1));
    }
}
// ----
