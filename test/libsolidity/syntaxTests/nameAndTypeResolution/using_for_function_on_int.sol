library D { function double(uint self) public returns (uint) { return 2*self; } }
contract C {
    using D for uint;
    function f(uint a) public returns (uint) {
        return a.double();
    }
}
// ----
// Warning: (12-79): Function state mutability can be restricted to pure
