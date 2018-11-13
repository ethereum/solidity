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
// SyntaxError: (223-235): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// SyntaxError: (245-257): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// SyntaxError: (267-280): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
