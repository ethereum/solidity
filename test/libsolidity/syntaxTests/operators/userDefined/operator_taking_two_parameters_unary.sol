type Int is int128;

using {bitnot as ~} for Int;

function bitnot(Int, Int) pure returns (Int) {}

contract C {
    function test() public pure {
        ~Int.wrap(1);
    }
}
// ----
// TypeError 1884: (66-76): Wrong parameters in operator definition. The function "bitnot" needs to have exactly one parameter of type Int to be used for the operator ~.
// TypeError 4907: (155-167): Built-in unary operator ~ cannot be applied to type Int. No matching user-defined operator found.
