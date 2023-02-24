function s(uint value) pure suffix returns (uint) { return value; }
function z(uint value) pure suffix returns (uint) { return value; }
function q(uint value) pure suffix returns (uint) { return value; }

contract C {
    address z;

    function q() internal {}

    function f() public {
        uint s;

        1 s;
        2 z;
        3 q;
    }
}
// ----
// Warning 2519: (298-304): This declaration shadows an existing declaration.
// Warning 2519: (222-231): This declaration shadows an existing declaration.
// Warning 2519: (238-262): This declaration shadows an existing declaration.
// TypeError 5704: (315-318): Integer cannot be used as a literal suffix.
