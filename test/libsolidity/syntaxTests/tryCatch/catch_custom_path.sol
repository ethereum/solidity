==== Source: A ====
error E(uint);
==== Source: B ====
import "A" as X;

contract C {
    function f() public {
        try this.f() {

        } catch X.E(uint x) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// UnimplementedFeatureError: NONE
// Warning 5667: (B:101-107): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
