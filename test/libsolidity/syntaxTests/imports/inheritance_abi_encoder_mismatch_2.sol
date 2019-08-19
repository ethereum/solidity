==== Source: A.sol ====
pragma experimental ABIEncoderV2;

contract A
{
    struct S { uint a; }
    S public s;
    function f(S memory _s) public returns (S memory,S memory) { }
}
==== Source: B.sol ====
import "./A.sol";
contract B is A { }
==== Source: C.sol ====
import "./B.sol";
contract C is B { }
// ----
// Warning: (A.sol:0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (B.sol:18-37): Contract "B" does not use the new experimental ABI encoder but wants to inherit from a contract which uses types that require it. Use "pragma experimental ABIEncoderV2;" for the inheriting contract as well to enable the feature.
