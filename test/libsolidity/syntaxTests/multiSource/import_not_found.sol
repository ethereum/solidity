==== Source: a ====
import "b";
contract C {}
// ----
// ParserError 6275: (a:0-11='import "b";'): Source "b" not found: File not supplied initially.
