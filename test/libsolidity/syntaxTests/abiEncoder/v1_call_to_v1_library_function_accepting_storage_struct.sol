==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }

    function set(Item storage _item) external view {}
}
==== Source: B ====
import "A";

contract Test {
    L.Item item;

    function foo() public view {
        L.set(item);
    }
}
// ----
