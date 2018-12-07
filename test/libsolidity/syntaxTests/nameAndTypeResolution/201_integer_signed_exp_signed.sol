contract test {
    function f() public { int x = 3; int y = 4; x ** y; }
    function g() public { int16 x = 3; uint8 y = 4; x ** y; }
    function h() public { uint8 x = 3; int16 y = 4; x ** y; }
}
// ----
// TypeError: (64-70): Operator ** not compatible with types int256 and int256. Exponentiation is not allowed for signed integer types.
// TypeError: (126-132): Operator ** not compatible with types int16 and uint8. Exponentiation is not allowed for signed integer types.
// TypeError: (188-194): Operator ** not compatible with types uint8 and int16. Exponentiation is not allowed for signed integer types.
