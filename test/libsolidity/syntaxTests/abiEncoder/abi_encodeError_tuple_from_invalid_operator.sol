contract C {
    event Ev();
    error Er();

    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    error e0();
    error e2(uint, uint);

    function h() public view {
        abi.encodeError(e2, (1, 1) + (2, 2));
        abi.encodeError(e0, Ev() / Er());
        abi.encodeError(e0, !());
    }
}
// ----
// TypeError 2271: (256-271): Built-in binary operator + cannot be applied to types tuple(int_const 1,int_const 1) and tuple(int_const 2,int_const 2).
// TypeError 9062: (256-271): Expected an inline tuple, not an expression of a tuple type.
// TypeError 2271: (302-313): Built-in binary operator / cannot be applied to types tuple() and error.
// TypeError 9062: (302-313): Expected an inline tuple, not an expression of a tuple type.
// TypeError 4907: (344-347): Built-in unary operator ! cannot be applied to type tuple().
// TypeError 9062: (344-347): Expected an inline tuple, not an expression of a tuple type.
