==== Source: s1.sol ====
error E(uint);
==== Source: s2.sol ====
import { E as Panic } from "s1.sol";
contract C {
  function a() public pure {
    require(false, Panic(1));
  }
}
// ====
// compileViaYul: also
// ----
// a() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"