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
// TypeError 8838: (635-636): The number cannot be converted to type bool accepted by the suffix function.
// TypeError 8838: (657-658): The number cannot be converted to type address accepted by the suffix function.
// TypeError 8838: (721-722): The number cannot be converted to type string memory accepted by the suffix function.
// TypeError 8838: (745-746): The number cannot be converted to type bytes memory accepted by the suffix function.
// TypeError 8838: (808-812): The number is out of range of type int8 accepted by the suffix function.
// TypeError 8838: (833-837): The number cannot be converted to type bool accepted by the suffix function.
// TypeError 8838: (858-862): The number cannot be converted to type address accepted by the suffix function.
// TypeError 8838: (925-929): The number cannot be converted to type string memory accepted by the suffix function.
// TypeError 8838: (952-956): The number cannot be converted to type bytes memory accepted by the suffix function.
// TypeError 8838: (979-983): The literal cannot be converted to type uint256 accepted by the suffix function.
// TypeError 8838: (1004-1008): The literal cannot be converted to type int8 accepted by the suffix function.
// TypeError 8838: (1068-1072): The literal cannot be converted to type address accepted by the suffix function.
// TypeError 2505: (1101-1114): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1124-1128): The literal cannot be converted to type string memory accepted by the suffix function.
// TypeError 8838: (1151-1155): The literal cannot be converted to type bytes memory accepted by the suffix function.
// TypeError 8838: (1178-1220): The address cannot be converted to type uint256 accepted by the suffix function.
// TypeError 8838: (1241-1283): The address cannot be converted to type int8 accepted by the suffix function.
// TypeError 8838: (1304-1346): The address cannot be converted to type bool accepted by the suffix function.
// TypeError 2505: (1487-1500): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (1510-1552): The address cannot be converted to type string memory accepted by the suffix function.
// TypeError 8838: (1575-1617): The address cannot be converted to type bytes memory accepted by the suffix function.
// TypeError 8838: (1640-1643): The number cannot be converted to type uint256 accepted by the suffix function.
// TypeError 8838: (1664-1667): The number cannot be converted to type int8 accepted by the suffix function.
// TypeError 8838: (1688-1691): The number cannot be converted to type bool accepted by the suffix function.
// TypeError 8838: (1712-1715): The number cannot be converted to type address accepted by the suffix function.
// TypeError 8838: (1777-1780): The number cannot be converted to type string memory accepted by the suffix function.
// TypeError 8838: (1803-1806): The number cannot be converted to type bytes memory accepted by the suffix function.
// TypeError 8838: (1829-1832): The literal cannot be converted to type uint256 accepted by the suffix function.
// TypeError 8838: (1853-1856): The literal cannot be converted to type int8 accepted by the suffix function.
// TypeError 8838: (1877-1880): The literal cannot be converted to type bool accepted by the suffix function.
// TypeError 8838: (1901-1904): The literal cannot be converted to type address accepted by the suffix function.
// TypeError 2505: (1932-1945): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (2032-2041): The literal cannot be converted to type uint256 accepted by the suffix function.
// TypeError 8838: (2062-2071): The literal cannot be converted to type int8 accepted by the suffix function.
// TypeError 8838: (2092-2101): The literal cannot be converted to type bool accepted by the suffix function.
// TypeError 8838: (2122-2131): The literal cannot be converted to type address accepted by the suffix function.
// TypeError 2505: (2165-2178): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
// TypeError 8838: (2188-2197): The literal cannot be converted to type string memory accepted by the suffix function.
