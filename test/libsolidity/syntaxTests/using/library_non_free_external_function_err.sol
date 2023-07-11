library L {
    function id_ext(uint x) external returns(uint) {
        return x;
    }
}

contract C {
    using L.id_ext for uint;
    function f(uint x) external {
        x.id_ext();
    }
}
// ----
// TypeError 4357: (115-123): Library name expected. If you want to attach a function, use '{...}'.
