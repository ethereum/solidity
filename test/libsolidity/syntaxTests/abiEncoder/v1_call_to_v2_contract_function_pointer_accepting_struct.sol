==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint x;
    }

    function get(Item memory _item) external {}
}
==== Source: B ====
import "A";

contract Test {
    function foo() public {
        C c = new C();
        function(C.Item memory) external ptr = c.get;
        ptr(C.Item(5));
    }
}
// ----
// TypeError 2443: (B:146-155): The type of this parameter, struct C.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
