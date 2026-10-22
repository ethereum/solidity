==== Source: A ====
pragma abicoder               v2;

library L {
    struct Item {
        uint x;
    }

    function set(Item storage _item) external view {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract Test {
    L.Item item;

    function foo() public view {
        L.set(item);
    }
}
// ====
// compileViaYul: false
// ----
// Warning 9511: (B:0-19): ABI coder v1 is deprecated and scheduled for removal. Use ABI coder v2 instead.
