function f(uint x) pure { }

contract C {
    using f for uint;
    function g(uint x) public pure {
        x.f();
    }
}
// ----
// TypeError 4357: (52-53): Library name expected. If you want to attach a function, use '{...}'.
