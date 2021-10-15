type MyAddress is address;
interface I {}
contract C {
    function f(MyAddress a) external {
    }
    function f(address a) external {
    }
}
contract D {
    function g(MyAddress a) external {
    }
}
contract E is D {
    function g(I a) external {
    }
}
// ----
// TypeError 9914: (104-142): Function overload clash during conversion to external types for arguments.
// TypeError 9914: (162-202): Function overload clash during conversion to external types for arguments.
