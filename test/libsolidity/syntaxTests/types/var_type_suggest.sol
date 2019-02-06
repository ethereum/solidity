contract C {
    function h() internal pure returns (uint, uint, uint) {
        return (1, 2, 4);
    }
    function g(uint x) internal pure returns (uint) {
        return x;
    }
    function f() internal pure {
        var si = -135;
        var i = 31415;
        var li = 31415999999999999999999999999999999999999999999999999999999999999999933**3;
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
// SyntaxError: (224-237): Use of the "var" keyword is disallowed. Use explicit declaration `int16 si = ...´ instead.
// SyntaxError: (247-260): Use of the "var" keyword is disallowed. Use explicit declaration `uint16 i = ...´ instead.
// TypeError: (279-353): Invalid rational int_const 3100...(204 digits omitted)...9237 (absolute value too large or division by zero).
// SyntaxError: (270-353): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// SyntaxError: (363-379): Use of the "var" keyword is disallowed. Use explicit declaration `string memory t = ...´ instead.
// SyntaxError: (389-399): Use of the "var" keyword is disallowed. Use explicit declaration `function (uint256) pure returns (uint256) g2 = ...´ instead.
// SyntaxError: (409-442): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// SyntaxError: (452-476): Use of the "var" keyword is disallowed. Use explicit declaration `(uint8 a, string memory b) = ...´ instead.
// SyntaxError: (486-503): Use of the "var" keyword is disallowed. Use explicit declaration `(uint256 x, , uint256 z) = ...´ instead.
// TypeError: (513-530): Different number of components on the left hand side (2) than on the right hand side (1).
// SyntaxError: (513-530): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// TypeError: (540-556): Different number of components on the left hand side (2) than on the right hand side (1).
// SyntaxError: (540-556): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// TypeError: (566-580): Different number of components on the left hand side (2) than on the right hand side (1).
// SyntaxError: (566-580): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
// TypeError: (590-605): Different number of components on the left hand side (2) than on the right hand side (1).
// SyntaxError: (590-605): Use of the "var" keyword is disallowed. Type cannot be expressed in syntax.
