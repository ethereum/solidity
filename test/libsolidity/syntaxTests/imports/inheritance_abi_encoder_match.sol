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
// Warning: (A.sol:0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (B.sol:0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (C.sol:0-33): Experimental features are turned on. Do not use experimental features on live deployments.
