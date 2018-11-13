contract C {
    function f() public pure returns (bool) {
        return abi.decode("abc", (uint)) == 2;
    }
}
