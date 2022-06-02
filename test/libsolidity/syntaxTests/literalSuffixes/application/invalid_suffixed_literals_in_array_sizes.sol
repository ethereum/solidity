function uintSuffix(uint x) pure returns (uint) {}
function int8Suffix(uint x) pure returns (int8) {}
function addressSuffix(uint x) pure returns (address) {}
function decimalSuffix(uint m, uint e) pure returns (uint) {}
function stringSuffix(uint x) pure returns (string memory) {}
function bytesSuffix(uint x) pure returns (bytes memory) {}

contract C {
    uint[42 uintSuffix] a;     // TODO: This should be an error too
    uint[42 int8Suffix] b;     // TODO: This should be an error too
    uint[42 addressSuffix] c;  // TODO: This should be an error too
    uint[42 decimalSuffix] d;  // TODO: This should be an error too
    uint[42 stringSuffix] e;   // TODO: This should be an error too
    uint[42 bytesSuffix] f;    // TODO: This should be an error too
}

contract D {
    uint[uintSuffix(42)] a;
    uint[int8Suffix(42)] b;
    uint[addressSuffix(42)] c;
    uint[decimalSuffix(42)] d;
    uint[stringSuffix(42)] e;
    uint[bytesSuffix(42)] f;
}
// ----
// TypeError 5462: (790-804): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (818-832): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (846-863): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (877-894): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (908-924): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (938-953): Invalid array length, expected integer literal or constant expression.
