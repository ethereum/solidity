contract test {
    function f() public {
        address(0x12).call("abc");
    }
}
// ----
// Warning 9302: (50-75): Return value of low-level calls not used.
