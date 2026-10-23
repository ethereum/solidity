==== ExternalSource: _nonNormalizedPaths//a.sol ====
==== ExternalSource: C/////c.sol=_nonNormalizedPaths/c.sol ====
==== ExternalSource: C/../////D/d.sol=_nonNormalizedPaths///d.sol ====
import {A} from "_nonNormalizedPaths//a.sol";
import {C} from "C/////c.sol";
import {D} from "C/../////D/d.sol";
contract Contract {
}
// ====
// targetContract: Contract
// ----
// constructor()
