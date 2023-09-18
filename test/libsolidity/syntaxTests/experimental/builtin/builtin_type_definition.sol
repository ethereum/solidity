pragma experimental solidity;

contract C {
    fallback() external {
        let v: void;

        let b: bool;
        bool.abs(bool.rep(b));

        let w: word;
        let i: integer;
        let u: unit;

        let f: fun(word, bool);
        b = f(w);

        let p: pair(bool, word);
        pair.first(p);
        pair.second(p);
    }
}
// ====
// EVMVersion: >=constantinople
// compileViaYul: true
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (48-348): Inferred type: () -> ()
// Info 4164: (56-58): Inferred type: ()
// Info 4164: (82-89): Inferred type: void
// Info 4164: (85-89): Inferred type: void
// Info 4164: (104-111): Inferred type: bool
// Info 4164: (107-111): Inferred type: bool
// Info 4164: (121-142): Inferred type: bool
// Info 4164: (121-129): Inferred type: word -> bool
// Info 4164: (121-125): Inferred type: bool
// Info 4164: (130-141): Inferred type: word
// Info 4164: (130-138): Inferred type: bool -> word
// Info 4164: (130-134): Inferred type: bool
// Info 4164: (139-140): Inferred type: bool
// Info 4164: (157-164): Inferred type: word
// Info 4164: (160-164): Inferred type: word
// Info 4164: (178-188): Inferred type: integer
// Info 4164: (181-188): Inferred type: integer
// Info 4164: (202-209): Inferred type: ()
// Info 4164: (205-209): Inferred type: ()
// Info 4164: (224-242): Inferred type: word -> bool
// Info 4164: (227-242): Inferred type: word -> bool
// Info 4164: (227-230): Inferred type: tfun((word, bool), word -> bool)
// Info 4164: (231-235): Inferred type: word
// Info 4164: (237-241): Inferred type: bool
// Info 4164: (252-260): Inferred type: bool
// Info 4164: (252-253): Inferred type: bool
// Info 4164: (256-260): Inferred type: bool
// Info 4164: (256-257): Inferred type: word -> bool
// Info 4164: (258-259): Inferred type: word
// Info 4164: (275-294): Inferred type: (bool, word)
// Info 4164: (278-294): Inferred type: (bool, word)
// Info 4164: (278-282): Inferred type: tfun((bool, word), (bool, word))
// Info 4164: (283-287): Inferred type: bool
// Info 4164: (289-293): Inferred type: word
// Info 4164: (304-317): Inferred type: bool
// Info 4164: (304-314): Inferred type: (bool, word) -> bool
// Info 4164: (304-308): Inferred type: ('bh:type, 'bi:type)
// Info 4164: (315-316): Inferred type: (bool, word)
// Info 4164: (327-341): Inferred type: word
// Info 4164: (327-338): Inferred type: (bool, word) -> word
// Info 4164: (327-331): Inferred type: ('bn:type, 'bo:type)
// Info 4164: (339-340): Inferred type: (bool, word)
