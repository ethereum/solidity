==== Source: A.sol ====
pragma experimental ABIEncoderV2;

contract A
{
    struct S { uint a; }
    S public s;
    function f(S memory _s) public returns (S memory,S memory) { }
}
==== Source: B.sol ====
pragma experimental ABIEncoderV2;

import "./A.sol";
contract B is A { }
==== Source: C.sol ====
pragma experimental ABIEncoderV2;

import "./B.sol";
contract C is B { }
// ----
