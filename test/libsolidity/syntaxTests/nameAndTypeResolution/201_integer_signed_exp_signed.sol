contract test {
    function f() public { int x = 3; int y = 4; x ** y; }
    function h() public { uint8 x = 3; int16 y = 4; x ** y; }
}
// ----
// TypeError: (64-70): Operator ** not compatible with types int256 and int256. Exponentiation power is not allowed to be a signed integer type.
// TypeError: (126-132): Operator ** not compatible with types uint8 and int16. Exponentiation power is not allowed to be a signed integer type.
// Warning: (126-132): The result type of the exponentiation operation is equal to the type of the first operand (uint8) ignoring the (larger) type of the second operand (int16) which might be unexpected. Silence this warning by either converting the first or the second operand to the type of the other.
