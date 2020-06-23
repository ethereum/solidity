contract test {
    function f() public {
        address(0x12).call{value: 2}("abc");
    }
}
// ----
// Warning 9302: (50-85): Return value of low-level calls not used.
