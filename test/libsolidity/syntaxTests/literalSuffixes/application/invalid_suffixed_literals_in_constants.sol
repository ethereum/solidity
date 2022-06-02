function uintSuffix(uint x) pure returns (uint) { return x; }
function int8Suffix(int8 x) pure returns (int8) { return x; }
function addressSuffix(address x) pure returns (address) { return x; }
function decimalSuffix(uint m, uint e) pure returns (uint) { return m + e; }
function stringSuffix(string memory x) pure returns (string memory) { return x; }
function bytesSuffix(bytes memory x) pure returns (bytes memory) { return x; }

contract C {
    uint constant a = 1 uintSuffix;
    int8 constant b = 1 int8Suffix;
    address constant c = 0x1234567890123456789012345678901234567890 addressSuffix;
    uint constant d = 1.1 decimalSuffix;
    string constant e = "a" stringSuffix;
    bytes constant f = hex"abcd" bytesSuffix;
}
// ----
// TypeError 8349: (469-481): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (505-517): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (544-600): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (624-641): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (667-683): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (708-729): Initial value for constant variable has to be compile-time constant.
