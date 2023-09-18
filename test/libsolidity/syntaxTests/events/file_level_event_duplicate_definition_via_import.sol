==== Source: A.sol ====
event E();
==== Source: B.sol ====
import "A.sol";

event E();
// ----
// DeclarationError 5883: (B.sol:17-27): Event with same name and parameter types defined twice.
