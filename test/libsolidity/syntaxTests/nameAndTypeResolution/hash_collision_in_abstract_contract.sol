// This contract used to throw
abstract contract D {
    function gsf() public {}
    function tgeo() public {}
}
contract C {
    D d;
    function g() public returns (uint) {
        d.d;
    }
}
// ----
// TypeError 1860: (31-113): Function signature hash collision for tgeo()
