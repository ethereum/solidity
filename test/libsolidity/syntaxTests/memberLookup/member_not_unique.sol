contract C {
    function a(uint256) public returns (uint) { return 1; }
    function a(uint8) public returns (uint) { return 1; }

    function f() public returns (C) { return this; }

    function g() internal returns (function(uint8) internal returns(uint))
    {
        return f().a;
    }
}
// ----
// TypeError: (282-287): Member "a" not unique after argument-dependent lookup in contract C.
