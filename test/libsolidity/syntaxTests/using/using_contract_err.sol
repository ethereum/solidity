contract C {
    function f(uint) external {
    }
}
interface I {
    function f(uint) external;
}

contract Test {
    using C for uint;
}
// ----
// TypeError 4357: (127-128): Library name expected. If you want to attach a function, use '{...}'.
