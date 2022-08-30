contract test {
    function f() public { int x = 3; int y = 4; x ** y; }
    function h() public { uint8 x = 3; int16 y = 4; x ** y; }
    function i() public { int16 x = 4; x ** -3; }
}
// ----
// TypeError 2271: (64-70): Binary operator ** not compatible with types int256 and int256. Exponentiation power is not allowed to be a signed integer type.
// TypeError 2271: (126-132): Binary operator ** not compatible with types uint8 and int16. Exponentiation power is not allowed to be a signed integer type.
// Warning 3149: (126-132): The result type of the exponentiation operation is equal to the type of the first operand (uint8) ignoring the (larger) type of the second operand (int16) which might be unexpected. Silence this warning by either converting the first or the second operand to the type of the other.
// TypeError 2271: (175-182): Binary operator ** not compatible with types int16 and int_const -3. Exponentiation power is not allowed to be a negative integer literal.
