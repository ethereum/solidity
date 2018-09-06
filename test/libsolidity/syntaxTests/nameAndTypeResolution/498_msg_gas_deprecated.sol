contract C {
    function f() public view returns (uint256 val) { return msg.gas; }
}
// ----
// TypeError: (73-80): "msg.gas" has been deprecated in favor of "gasleft()"
