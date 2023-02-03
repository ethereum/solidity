function f(uint x) pure { }

using f for uint;

contract C {
    function g(uint x) public pure {
        x.f();
    }
}
// ----
// TypeError 4357: (35-36): Library name expected. If you want to attach a function, use '{...}'.
