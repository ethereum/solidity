interface I1 {
    function f() external pure;
}

interface I2 {
    function g() external pure;
}

contract C {
    function test(bool b) public returns(bytes4) {
        (b ? I1.f : I2.g).selector;
    }
}
// ----
// TypeError 9717: (177-181): Invalid mobile type in true expression.
// TypeError 3703: (184-188): Invalid mobile type in false expression.
