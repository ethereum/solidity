==== ExternalSource: ./a.sol=_sourceNameStartingWithDots/dot_a.sol ====
==== ExternalSource: ../b.sol=_sourceNameStartingWithDots/dot_dot_b.sol ====
==== ExternalSource: _sourceNameStartingWithDots/dir/a.sol ====
==== ExternalSource: _sourceNameStartingWithDots/b.sol ====
==== ExternalSource: _sourceNameStartingWithDots/dir/contract.sol ====
import {A, B} from "_sourceNameStartingWithDots/dir/contract.sol";
contract Contract {
}
// ====
// targetContract: Contract
// ----
// constructor()
