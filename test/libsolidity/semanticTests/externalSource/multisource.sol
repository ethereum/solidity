==== ExternalSource: _external/external.sol ====
==== Source: s1.sol ====
import {External} from "_external/external.sol";
contract S1 {
}
==== Source: s2.sol ====
import {S1} from "s1.sol";
contract C {
}
// ====
// compileViaYul: also
// ----
// constructor()

