pragma abicoder               v2;

library Lib {
    struct Items {
        mapping (uint => uint) a;
    }

    function get() public returns (Items storage x) {
        assembly { x.slot := 123 }
    }
}

contract C {
    function f() public returns(uint256 slot) {
        Lib.Items storage ptr = Lib.get();
        assembly { slot := ptr.slot }
    }
}
// ----
// library: Lib
// f() -> 123
