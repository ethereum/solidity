pragma experimental solidity;

class T: K {
    function new() -> T;
}

contract C {
    fallback() external {
        let k = K.new();
    }
}

// ----
// () ->
