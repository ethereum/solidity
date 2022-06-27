==== Source: A ====
pragma abicoder               v2;

contract C {
    struct Item {
        uint x;
    }

    function set(uint _x, string memory _y, Item memory _item, bool _z) external view {}
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract Test {
    function foo() public view {
        C(address(0x00)).set({_item: C.Item(50), _z: false, _y: "abc", _x: 30});
    }
}
// ----
// TypeError 2443: (B:119-129): The type of this parameter, struct C.Item memory, is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
