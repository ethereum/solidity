==== Source: A ====
error E(uint);
==== Source: B ====
import {E as Error} from "A";

contract C {
    function f() public {
        try this.f() {

        } catch Error(uint x) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 2943: (B:104-136): Expected `catch Error(string memory ...) { ... }`.
