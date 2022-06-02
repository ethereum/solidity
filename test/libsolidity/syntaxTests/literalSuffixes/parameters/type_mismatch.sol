function uintSuffix(uint) pure returns (uint) { return 1; }
function int8Suffix(int8) pure returns (uint) { return 1; }
function boolSuffix(bool) pure returns (uint) { return 1; }
function addressSuffix(address) pure returns (uint) { return 1; }
function decimalSuffix(uint, uint) pure returns (uint) { return 1; }
function stringSuffix(string memory) pure returns (uint) { return 1; }
function bytesSuffix(bytes memory) pure returns (uint) { return 1; }

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
// TypeError 8838: (586-598): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (608-623): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (672-686): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (696-709): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (759-774): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (784-799): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (809-827): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (876-893): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (903-919): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (930-945): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (955-970): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1019-1037): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 4778: (1047-1065): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1075-1092): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1102-1118): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1129-1182): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1192-1245): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1255-1308): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 4778: (1395-1451): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1461-1516): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1526-1580): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1591-1605): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1615-1629): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1639-1653): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1663-1680): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1728-1744): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1754-1769): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1780-1794): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1804-1818): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1828-1842): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (1852-1869): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 4778: (1879-1896): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1983-2003): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2013-2033): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2043-2063): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (2073-2096): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 4778: (2106-2129): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (2139-2161): The type of the literal cannot be converted to the parameter of the suffix function.
