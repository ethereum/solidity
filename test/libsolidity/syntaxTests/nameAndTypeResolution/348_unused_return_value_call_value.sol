contract test {
    function f() public {
        address(0x12).call.value(2)("abc");
        address(0x12).call{value: 2}("abc");
    }
}
// ----
// Warning: (50-74): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning: (50-84): Return value of low-level calls not used.
// Warning: (94-129): Return value of low-level calls not used.
