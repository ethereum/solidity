==== ExternalSource: _non_normalized_paths//a.sol ====
==== ExternalSource: C/////c.sol=_non_normalized_paths/c.sol ====
==== ExternalSource: C/../////D/d.sol=_non_normalized_paths///d.sol ====
import {A} from "_non_normalized_paths//a.sol";
import {C} from "C/////c.sol";
import {D} from "C/../////D/d.sol";
contract Contract {
}
// ----
// constructor()
