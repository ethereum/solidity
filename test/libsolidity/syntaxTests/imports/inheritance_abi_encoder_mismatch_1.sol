==== Source: A.sol ====
pragma abicoder               v2;

contract A
{
    struct S { uint a; }
    S public s;
    function f(S memory _s) public returns (S memory,S memory) { }
}
==== Source: B.sol ====
pragma abicoder               v2;

import "./A.sol";
contract B is A { }
==== Source: C.sol ====
pragma abicoder v1;
import "./B.sol";
contract C is B { }
// ----
// TypeError 6594: (C.sol:38-57): Contract "C" does not use ABI coder v2 but wants to inherit from a contract which uses types that require it. Use "pragma abicoder v2;" for the inheriting contract as well to enable the feature.
