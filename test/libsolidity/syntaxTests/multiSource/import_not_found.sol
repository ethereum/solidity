==== Source: a ====
import "b";
contract C {}
// ----
// ParserError 6275: (a:0-11): Source "b" not found: File not supplied initially.
