==== ExternalSource: _external/external.sol ====
==== ExternalSource: _external/other_external.sol ====
import {External} from "_external/external.sol";
import {OtherExternal} from "_external/other_external.sol";
contract C {
}
// ====
// compileViaYul: also
// ----
// constructor()
