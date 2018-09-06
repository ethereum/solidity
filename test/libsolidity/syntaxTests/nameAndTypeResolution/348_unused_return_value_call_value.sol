contract test {
    function f() public {
        address(0x12).call.value(2)("abc");
    }
}
// ----
// Warning: (50-84): Return value of low-level calls not used.
