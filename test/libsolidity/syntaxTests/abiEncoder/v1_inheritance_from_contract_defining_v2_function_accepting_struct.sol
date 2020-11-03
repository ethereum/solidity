==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint x;
    }

    function get(Item memory) external view {}
}
==== Source: B ====
import "A";

contract D is C {}
// ----
// TypeError 6594: (B:13-31): Contract "D" does not use ABI coder v2 but wants to inherit from a contract which uses types that require it. Use "pragma abicoder v2;" for the inheriting contract as well to enable the feature.
