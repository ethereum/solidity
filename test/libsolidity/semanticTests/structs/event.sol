pragma abicoder v2;

struct Item {uint x;}
library L {
    event Ev(Item);
    function o() public { emit L.Ev(Item(1)); }
}
contract C {
    function f() public {
        L.o();
    }
}
// ----
// library: L
// f() ->
// ~ emit Ev((uint256)): 0x01
