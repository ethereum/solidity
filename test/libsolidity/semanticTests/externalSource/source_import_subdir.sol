==== ExternalSource: _external/import_with_subdir.sol ====
==== ExternalSource: subdir/import.sol=_external/subdir/import.sol ====
==== ExternalSource: sub_external.sol=_external/subdir/sub_external.sol ====
import {SubExternal} from "sub_external.sol";
contract C {
}
// ====
// compileViaYul: also
// ----
// constructor()
