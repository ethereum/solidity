==== Source: A.sol ====
contract A {}
==== Source: B.sol ====
pragma experimental solidity;
import "A.sol";
contract B {
    A a;
}
==== Source: C.sol ====
pragma experimental solidity;
import "A.sol";
contract C {
    A a;
}
==== Source: D.sol ====
pragma experimental solidity;
import "A.sol";
contract D {
    A a;
}
// ====
// EVMVersion: >=constantinople
// ----
// ParserError 2141: (B.sol:0-29): File declares "pragma experimental solidity". If you want to enable the experimental mode, all source units must include the pragma.
// ParserError 2141: (C.sol:0-29): File declares "pragma experimental solidity". If you want to enable the experimental mode, all source units must include the pragma.
// ParserError 2141: (D.sol:0-29): File declares "pragma experimental solidity". If you want to enable the experimental mode, all source units must include the pragma.
