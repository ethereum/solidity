function asUint(uint x) pure returns (uint) {}
function asSint(int x) pure returns (uint) {}
function asUdec(uint m, uint e) pure returns (uint) {}
function asSdec(int m, uint e) pure returns (uint) {}

contract C {
    function f() public pure {
        1 asUint;
        1 asSint;
        1 asUdec;
        1 asSdec;

        -1 asUint;
        -1 asSint;
        -1 asUdec;
        -1 asSdec;

        1.1 asUdec;
        1.1 asSdec;

        -1.1 asUdec;
        -1.1 asSdec;
    }
}
// ----
// TypeError 4907: (328-337): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (347-356): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (366-375): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (385-394): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (446-457): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (467-478): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
