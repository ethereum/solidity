==== ExternalSource: _relative_imports/dir/contract.sol ====
==== ExternalSource: _relative_imports/dir/a.sol ====
==== ExternalSource: _relative_imports/dir/B/b.sol ====
==== ExternalSource: _relative_imports/c.sol ====
==== ExternalSource: _relative_imports/D/d.sol ====
==== ExternalSource: _relative_imports/dir/G/g.sol ====
==== ExternalSource: _relative_imports/h.sol ====
import {A, B, C, D, G, H, Contract} from "_relative_imports/dir/contract.sol";
contract CC {
}
// ====
// compileViaYul: also
// ----
// constructor()
