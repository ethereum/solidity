contract A {
    uint public x;
    function f(uint a) public {
        x = a;
    }
}

contract B is A {
    uint public y;
    function g(uint a) public {
        f(x + a);
        y = x + 1;
    }
}

contract C is B {
    uint public w;
    function h(uint a) public {
        g(a);
        w = x + y;
    }
}

contract D is A, B, C layout at 42 {
    uint public z;
    function test() public returns (uint, uint, uint, uint) {
        h(1);
        z = w + 2;
        return (x, y, w, z);
    }
}
// ----
// test() -> 1, 2, 3, 5
// gas irOptimized: 110109
// gas legacy: 111881
// gas legacyOptimized: 110945
// gas ssaCFGOptimized: 110203
// x() -> 1
// y() -> 2
// w() -> 3
// z() -> 5
