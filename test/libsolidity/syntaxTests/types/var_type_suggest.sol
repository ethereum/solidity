contract C {
    function h() internal pure returns (uint, uint, uint) {
        return (1, 2, 4);
    }
    function g(uint x) internal pure returns (uint) {
        return x;
    }
    function f() internal pure {
        var i = 31415;
        var t = "string";
        var g2 = g;
        var myblockhash = block.blockhash;
        var (a, b) = (2, "troi");
        var (x,, z) = h();
    }
}
// ----
// SyntaxError: (224-237): Use of the "var" keyword is disallowed. Use explicit declaration `uint16 i = ...´ instead.
// SyntaxError: (247-263): Use of the "var" keyword is disallowed. Use explicit declaration `string memory t = ...´ instead.
// SyntaxError: (273-283): Use of the "var" keyword is disallowed. Use explicit declaration `function (uint256) pure returns (uint256) g2 = ...´ instead.
// SyntaxError: (293-326): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// SyntaxError: (336-360): Use of the "var" keyword is disallowed. Use explicit declaration `(uint8 a, string memory b) = ...´ instead.
// SyntaxError: (370-387): Use of the "var" keyword is disallowed. Use explicit declaration `(uint256 x, , uint256 z) = ...´ instead.
