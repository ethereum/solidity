function asUint(uint x) pure suffix returns (uint) {}
function asSint(int x) pure suffix returns (uint) {}
function asUdec(uint m, uint e) pure suffix returns (uint) {}
function asSdec(int m, uint e) pure suffix returns (uint) {}

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
// TypeError 4907: (356-365): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (375-384): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (394-403): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (413-422): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (474-485): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
// TypeError 4907: (495-506): Built-in unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
