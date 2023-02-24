function uintSuffix(uint x) pure suffix returns (uint) {}
function int8Suffix(uint x) pure suffix returns (int8) {}
function addressSuffix(uint x) pure suffix returns (address) {}
function decimalSuffix(uint m, uint e) pure suffix returns (uint) {}
function stringSuffix(uint x) pure suffix returns (string memory) {}
function bytesSuffix(uint x) pure suffix returns (bytes memory) {}

contract C {
    uint[42 uintSuffix] a;
    uint[42 int8Suffix] b;
    uint[42 addressSuffix] c;
    uint[42 decimalSuffix] d;
    uint[42 stringSuffix] e;
    uint[42 bytesSuffix] f;
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
// TypeError 5462: (408-421): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (435-448): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (462-478): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (492-508): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (522-537): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (551-565): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a denomination.
// TypeError 5462: (595-609): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (623-637): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (651-668): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (682-699): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (713-729): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (743-758): Invalid array length, expected integer literal or constant expression.
