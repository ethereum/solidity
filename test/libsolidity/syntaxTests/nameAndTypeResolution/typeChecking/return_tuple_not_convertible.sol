contract test {
    function f() public returns (uint256 r, uint8) { return ((12, "")); }
}
// ----
// TypeError: (76-86): Return argument type tuple(int_const 12,literal_string "") is not implicitly convertible to expected type tuple(uint256,uint8).
