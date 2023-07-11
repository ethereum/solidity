type Int is int32;

using {add as +, add as +} for Int global;

function add(Int, Int) pure returns(Int) {}

function f(int32 a, int32 b) pure {
    Int.wrap(a) + Int.wrap(b);
}
