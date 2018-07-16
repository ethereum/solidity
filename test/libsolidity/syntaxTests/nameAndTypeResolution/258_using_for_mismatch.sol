library D { function double(bytes32 self) public returns (uint) { return 2; } }
contract C {
    using D for uint;
    function f(uint a) public returns (uint) {
        return a.double();
    }
}
// ----
// TypeError: (177-185): Member "double" not found or not visible after argument-dependent lookup in uint256.
