type Int is int;

using {C.add as +} for Int global;

contract C {
    function add(Int, Int) public pure returns (Int) {
        return 0;
    }
}
// ----
// TypeError 4167: (25-30): Only file-level functions and library functions can be attached to a type in a "using" statement
