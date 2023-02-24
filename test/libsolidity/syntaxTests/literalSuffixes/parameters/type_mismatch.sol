function uintSuffix(uint) pure suffix returns (uint) { return 1; }
function int8Suffix(int8) pure suffix returns (uint) { return 1; }
function boolSuffix(bool) pure suffix returns (uint) { return 1; }
function addressSuffix(address) pure suffix returns (uint) { return 1; }
function decimalSuffix(uint, uint) pure suffix returns (uint) { return 1; }
function stringSuffix(string memory) pure suffix returns (uint) { return 1; }
function bytesSuffix(bytes memory) pure suffix returns (uint) { return 1; }

contract C {
    function f() public pure {
        1 uintSuffix;       // allowed
        1 int8Suffix;       // allowed
        1 boolSuffix;
        1 addressSuffix;
        1 decimalSuffix;    // allowed
        1 stringSuffix;
        1 bytesSuffix;

        1024 uintSuffix;    // allowed
        1024 int8Suffix;
        1024 boolSuffix;
        1024 addressSuffix;
        1024 decimalSuffix; // allowed
        1024 stringSuffix;
        1024 bytesSuffix;

        true uintSuffix;
        true int8Suffix;
        true boolSuffix;    // allowed
        true addressSuffix;
        true decimalSuffix;
        true stringSuffix;
        true bytesSuffix;

        0x1234567890123456789012345678901234567890 uintSuffix;
        0x1234567890123456789012345678901234567890 int8Suffix;
        0x1234567890123456789012345678901234567890 boolSuffix;
        0x1234567890123456789012345678901234567890 addressSuffix; // allowed
        0x1234567890123456789012345678901234567890 decimalSuffix;
        0x1234567890123456789012345678901234567890 stringSuffix;
        0x1234567890123456789012345678901234567890 bytesSuffix;

        1.1 uintSuffix;
        1.1 int8Suffix;
        1.1 boolSuffix;
        1.1 addressSuffix;
        1.1 decimalSuffix; // allowed
        1.1 stringSuffix;
        1.1 bytesSuffix;

        "a" uintSuffix;
        "a" int8Suffix;
        "a" boolSuffix;
        "a" addressSuffix;
        "a" decimalSuffix;
        "a" stringSuffix;  // allowed
        "a" bytesSuffix;   // allowed

        hex"abcd" uintSuffix;
        hex"abcd" int8Suffix;
        hex"abcd" boolSuffix;
        hex"abcd" addressSuffix;
        hex"abcd" decimalSuffix;
        hex"abcd" stringSuffix;
        hex"abcd" bytesSuffix;   // allowed
    }
}
// ----
// TypeError 8838: (635-647): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (657-672): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (721-735): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (745-758): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (808-823): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (833-848): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (858-876): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (925-942): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (952-968): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (979-994): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1004-1019): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1068-1086): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 2505: (1096-1114): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1124-1141): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1151-1167): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1178-1231): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1241-1294): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1304-1357): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 2505: (1444-1500): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1510-1565): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1575-1629): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1640-1654): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1664-1678): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1688-1702): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1712-1729): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1777-1793): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1803-1818): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1829-1843): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1853-1867): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1877-1891): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1901-1918): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 2505: (1928-1945): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (2032-2052): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2062-2082): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2092-2112): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2122-2145): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 2505: (2155-2178): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (2188-2210): The type of the literal cannot be converted to the parameter of the suffix function.
