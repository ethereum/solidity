==== Source: A ====
pragma experimental ABIEncoderV2;

library L {
    struct Item {
        uint x;
    }

    function get(Item storage _item) external view {}
}
==== Source: B ====
import "A";

contract Test {
    L.Item item;

    function foo() public view {
        L.get(item);
    }
}
// ----
