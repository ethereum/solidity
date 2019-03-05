contract C {
    function h() internal pure returns (uint, uint, uint) {
        return (1, 2, 4);
    }
    function g(uint x) internal pure returns (uint) {
        return x;
    }
    function f() internal pure {
        var s = -31415;
        var i = 31415;
        var t = "string";
        var g2 = g;
        var myblockhash = block.blockhash;
        var (a, b) = (2, "troi");
        var (x,, z) = h();
        var (c, d) = ("");
        var (k, l) = (2);
        var (m, n) = 1;
        var (o, p) = "";
    }
}
// ----
// SyntaxError: (224-238): Use of the "var" keyword is disallowed. Use explicit declaration `int16 s = ...´ instead.
// SyntaxError: (248-261): Use of the "var" keyword is disallowed. Use explicit declaration `uint16 i = ...´ instead.
// SyntaxError: (271-287): Use of the "var" keyword is disallowed. Use explicit declaration `string memory t = ...´ instead.
// SyntaxError: (297-307): Use of the "var" keyword is disallowed. Use explicit declaration `function (uint256) pure returns (uint256) g2 = ...´ instead.
// SyntaxError: (317-350): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// SyntaxError: (360-384): Use of the "var" keyword is disallowed. Use explicit declaration `(uint8 a, string memory b) = ...´ instead.
// SyntaxError: (394-411): Use of the "var" keyword is disallowed. Use explicit declaration `(uint256 x, , uint256 z) = ...´ instead.
// TypeError: (421-438): Different number of components on the left hand side (2) than on the right hand side (1).
