==== Source: s1.sol ====
error E(uint);
==== Source: s2.sol ====
import "s1.sol" as S;
==== Source: s3.sol ====
import "s1.sol" as S;
import "s2.sol" as T;
import "s1.sol";
contract C {
  function x() public pure {
    revert E(1);
  }
  function y() public pure {
    revert S.E(2);
  }
  function z() public pure {
    revert T.S.E(3);
  }
}
// ----
// x() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"
// y() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000002"
// z() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000003"
