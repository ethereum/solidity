==== Source: A.sol ====
function uintSuffix(uint x) pure suffix returns (uint) {}
function int8Suffix(uint x) pure suffix returns (int8) {}
function addressSuffix(uint x) pure suffix returns (address) {}
function decimalSuffix(uint m, uint e) pure suffix returns (uint) {}
function stringSuffix(uint x) pure suffix returns (string memory) {}
function bytesSuffix(uint x) pure suffix returns (bytes memory) {}

==== Source: B.sol ====
import "A.sol" as A;
import "A.sol";

contract C {
    uint[42 uintSuffix] a;
    uint[42 int8Suffix] b;
    uint[42 addressSuffix] c;
    uint[42 decimalSuffix] d;
    uint[42 stringSuffix] e;
    uint[42 bytesSuffix] f;

    uint[42 A.uintSuffix] a;
    uint[42 A.int8Suffix] b;
    uint[42 A.addressSuffix] c;
    uint[42 A.decimalSuffix] d;
    uint[42 A.stringSuffix] e;
    uint[42 A.bytesSuffix] f;
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
// DeclarationError 2333: (B.sol:227-250): Identifier already declared.
// DeclarationError 2333: (B.sol:256-279): Identifier already declared.
// DeclarationError 2333: (B.sol:285-311): Identifier already declared.
// DeclarationError 2333: (B.sol:317-343): Identifier already declared.
// DeclarationError 2333: (B.sol:349-374): Identifier already declared.
// DeclarationError 2333: (B.sol:380-404): Identifier already declared.
// TypeError 5462: (B.sol:60-73): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:87-100): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:114-130): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:144-160): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:174-189): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:203-217): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:232-247): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:261-276): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:290-308): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:322-340): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:354-371): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:385-401): Invalid array length, expected integer literal or constant expression. A suffixed literal is not a constant expression unless the suffix is a subdenomination.
// TypeError 5462: (B.sol:431-445): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (B.sol:459-473): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (B.sol:487-504): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (B.sol:518-535): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (B.sol:549-565): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (B.sol:579-594): Invalid array length, expected integer literal or constant expression.
