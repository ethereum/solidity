type Int is int128;

using {bitnot as ~} for Int global;

function bitnot(Int, Int) pure returns (Int) {}

contract C {
    function test() public pure {
        ~Int.wrap(1);
    }
}
// ----
// TypeError 1884: (73-83): Wrong parameters in operator definition. The function "bitnot" needs to have exactly one parameter of type Int to be used for the operator ~.
// TypeError 4907: (162-174): Built-in unary operator ~ cannot be applied to type Int. No matching user-defined operator found.
