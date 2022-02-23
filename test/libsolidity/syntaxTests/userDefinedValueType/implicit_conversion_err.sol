type MyInt is int;
function f(int a) pure returns (int) {
    MyInt b = a;

    int c = b;

    address d = b;

    MyInt e = d;

    uint x = 0;
    MyInt y = MyInt(x);

    return e;
}
// ----
// TypeError 9574: (62-73): Type int256 is not implicitly convertible to expected type MyInt.
// TypeError 9574: (80-89): Type MyInt is not implicitly convertible to expected type int256.
// TypeError 9574: (96-109): Type MyInt is not implicitly convertible to expected type address.
// TypeError 9574: (116-127): Type address is not implicitly convertible to expected type MyInt.
// TypeError 9640: (160-168): Explicit type conversion not allowed from "uint256" to "MyInt".
// TypeError 6359: (182-183): Return argument type MyInt is not implicitly convertible to expected type (type of first return variable) int256.
