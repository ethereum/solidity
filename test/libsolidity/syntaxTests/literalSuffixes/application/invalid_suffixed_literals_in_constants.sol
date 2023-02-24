function uintSuffix(uint x) pure suffix returns (uint) { return x; }
function int8Suffix(int8 x) pure suffix returns (int8) { return x; }
function addressSuffix(address x) pure suffix returns (address) { return x; }
function decimalSuffix(uint m, uint e) pure suffix returns (uint) { return m + e; }
function stringSuffix(string memory x) pure suffix returns (string memory) { return x; }
function bytesSuffix(bytes memory x) pure suffix returns (bytes memory) { return x; }

contract C {
    uint constant a = 1 uintSuffix;
    int8 constant b = 1 int8Suffix;
    address constant c = 0x1234567890123456789012345678901234567890 addressSuffix;
    uint constant d = 1.1 decimalSuffix;
    string constant e = "a" stringSuffix;
    bytes constant f = hex"abcd" bytesSuffix;
}
// ----
// TypeError 8349: (511-523): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (547-559): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (586-642): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (666-683): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (709-725): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (750-771): Initial value for constant variable has to be compile-time constant.
