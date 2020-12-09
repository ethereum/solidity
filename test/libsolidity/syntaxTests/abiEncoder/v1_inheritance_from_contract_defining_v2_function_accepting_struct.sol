==== Source: A ====
pragma abicoder               v2;

contract C {
    struct Item {
        uint x;
    }

    function get(Item memory) external view {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract D is C {}
// ----
// TypeError 6594: (B:33-51): Contract "D" does not use ABI coder v2 but wants to inherit from a contract which uses types that require it. Use "pragma abicoder v2;" for the inheriting contract as well to enable the feature.
