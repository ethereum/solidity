contract C {
    function value(uint256) public returns (uint) { return 1; }
    function value(uint8) public returns (uint) { return 1; }

    function f() public returns (C) { return this; }

    function g() internal returns (function(uint8) internal returns(uint))
    {
        return f().value;
    }
}
// ----
// TypeError: (290-299): Member "value" not unique after argument-dependent lookup in contract C - did you forget the "payable" modifier?
