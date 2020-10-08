==== Source: A ====
pragma experimental ABIEncoderV2;

contract C {
    struct Item {
        uint x;
    }

    function set(uint _x, string memory _y, Item memory _item, bool _z) external view {}
}
==== Source: B ====
import "A";

contract Test {
    function foo() public view {
        C(0x00).set({_item: C.Item(50), _z: false, _y: "abc", _x: 30});
    }
}
// ----
// TypeError 2443: (B:90-100): The type of this parameter, struct C.Item, is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
