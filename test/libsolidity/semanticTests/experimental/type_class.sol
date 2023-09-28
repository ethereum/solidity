pragma experimental solidity;

type word = __builtin("word");
type bool = __builtin("bool");

type Cat = word;
type Dog = word;

class Self: Animal {
    function new() -> Self;
    function alive(self: Self) -> bool;
}

instantiation Cat: Animal {
    function new() -> Cat {
        let c;
        return c;
    }

    function alive(self: Cat) -> bool {
        // TODO: Boolean literals or operators not implemented.
        let w;
        assembly {
            w := 1
        }
        return bool.abs(w);
    }
}

instantiation Dog: Animal {
    function new() -> Dog {
        let d: Dog;
        return d;
    }

    function alive(self: Dog) -> bool {
        let b: bool;
        return b;
    }
}

contract C {
    fallback() external {
        let boolResult1: bool;
        let boolResult2: bool;

        let c: Cat = Animal.new();
        boolResult1 = Animal.alive(c);

        let d: Dog = Animal.new();
        boolResult2 = Animal.alive(d);

        let wordResult1 = bool.rep(boolResult1);
        let wordResult2 = bool.rep(boolResult2);
        assembly {
            mstore(0, wordResult1)
            mstore(32, wordResult2)
            return(0, 64)
        }
    }
}

// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// () -> 1, 0
