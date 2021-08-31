// Test that the parser workaround is not breaking.
struct error {uint a;}
contract C {
    error x;
}
// ----
