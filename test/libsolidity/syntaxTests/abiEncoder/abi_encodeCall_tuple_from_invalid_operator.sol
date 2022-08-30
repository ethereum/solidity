contract C {
    event Ev();
    error Er();

    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f0() public {}
    function f2(uint, uint) public {}

    function h() public view {
        abi.encodeCall(this.f2, (1, 1) + (2, 2));
        abi.encodeCall(this.f0, Ev() / Er());
        abi.encodeCall(this.f0, !());
    }
}
// ----
// TypeError 2271: (284-299): Binary operator + not compatible with types tuple(int_const 1,int_const 1) and tuple(int_const 2,int_const 2).
// TypeError 9062: (284-299): Expected an inline tuple, not an expression of a tuple type.
// TypeError 2271: (334-345): Binary operator / not compatible with types tuple() and tuple().
// TypeError 9062: (334-345): Expected an inline tuple, not an expression of a tuple type.
// TypeError 4907: (380-383): Unary operator ! cannot be applied to type tuple().
// TypeError 9062: (380-383): Expected an inline tuple, not an expression of a tuple type.
