function s(uint value) pure returns (uint) { return value; }
function z(uint value) pure returns (uint) { return value; }
function q(uint value) pure returns (uint) { return value; }

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
// Warning 2519: (277-283): This declaration shadows an existing declaration.
// Warning 2519: (201-210): This declaration shadows an existing declaration.
// Warning 2519: (217-241): This declaration shadows an existing declaration.
// TypeError 4438: (294-297): The literal suffix needs to be a pre-defined suffix or a file-level function.
// TypeError 4438: (307-310): The literal suffix needs to be a pre-defined suffix or a file-level function.
// TypeError 4438: (320-323): The literal suffix needs to be a pre-defined suffix or a file-level function.
