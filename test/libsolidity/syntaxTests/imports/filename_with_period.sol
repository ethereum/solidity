==== Source: a/.b.sol ====
contract B {}
==== Source: a/a.sol ====
import ".b.sol"; contract A is B {}
// ----
// ParserError: (a/a.sol:0-16): Source ".b.sol" not found: File not supplied initially.
