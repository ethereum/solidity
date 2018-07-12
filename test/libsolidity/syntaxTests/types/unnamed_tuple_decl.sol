pragma solidity ^0.4.20;

contract C {
    function f() internal pure {}
    function g() internal pure returns (uint) { return 1; }
    function h() internal pure returns (uint, uint) { return (1, 2); }

    function test() internal pure {
        var () = f();
        var () = g();
        var (,) = h();
    }
}

// ----
// SyntaxError: (249-261): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// SyntaxError: (271-283): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// SyntaxError: (293-306): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// TypeError: (271-283): Too many components (1) in value for variable assignment (0) needed
