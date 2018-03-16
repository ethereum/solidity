contract C {
    function f() public returns (address) {
        return address(f);
    }
}
