contract A {
    uint public x;
    function xSlot() public view returns (uint xs) {
        assembly {
            xs := x.slot
        }
    }
}

contract B is A layout at 5 {
    uint public y;
    function xAndYSlots() public view returns (uint xs, uint ys) {
        assembly {
            xs := x.slot
            ys := y.slot
        }
    }
}

contract C is A layout at 9 {
    uint public z;
    function xAndZSlots() public view returns (uint xs, uint zs) {
        assembly {
            xs := x.slot
            zs := z.slot
        }
    }
}

contract Test {
    A a = new A();
    B b = new B();
    C c = new C();
    function contractASlots() public view returns (uint) {
        return a.xSlot();
    }
    function contractBSlots() public view returns (uint, uint) {
        return b.xAndYSlots();
    }
    function contractCSlots() public view returns (uint, uint) {
        return c.xAndZSlots();
    }
}
// ====
// targetContract: Test
// ----
// contractASlots() -> 0
// contractBSlots() -> 5, 6
// contractCSlots() -> 9, 10
