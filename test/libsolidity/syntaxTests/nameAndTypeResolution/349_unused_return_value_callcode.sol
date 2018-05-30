contract test {
    function f() public {
        address(0x12).callcode("abc");
    }
}
// ----
// Warning: (50-79): Return value of low-level calls not used.
// Warning: (50-72): "callcode" has been deprecated in favour of "delegatecall".
