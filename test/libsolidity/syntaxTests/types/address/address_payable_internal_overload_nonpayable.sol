contract C {
    function f(address payable) internal pure {}
    function f(address) internal pure returns (uint) {}
    function g() internal pure {
        address a = address(0);
        uint b = f(a); // TODO: should this be valid?
        b;
    }
}
// ----
