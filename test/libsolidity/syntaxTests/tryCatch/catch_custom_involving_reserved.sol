==== Source: A ====
error E(uint);
==== Source: B ====
import "A" as Error;

contract C {
    function f() public {
        try this.f() {

        } catch Error.E(uint x) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// ParserError 2314: (B:106-107): Expected '(' but got '.'
