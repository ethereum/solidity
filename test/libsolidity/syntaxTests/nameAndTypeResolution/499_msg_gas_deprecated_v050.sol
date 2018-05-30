pragma experimental "v0.5.0";
contract C {
    function f() public returns (uint256 val) { return msg.gas; }
}
// ----
// TypeError: (98-105): "msg.gas" has been deprecated in favor of "gasleft()"
