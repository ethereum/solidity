pragma experimental solidity;

type void = __builtin("void");

type bool = __builtin("bool");
type word = __builtin("word");
type integer = __builtin("integer");
type unit = __builtin("unit");

type fun(T, U) = __builtin("fun");
type pair(T, U) = __builtin("pair");

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
// Info 4164: (31-61): Inferred type: void
// Info 4164: (63-93): Inferred type: bool
// Info 4164: (94-124): Inferred type: word
// Info 4164: (125-161): Inferred type: integer
// Info 4164: (162-192): Inferred type: ()
// Info 4164: (194-228): Inferred type: tfun(('l:type, 'm:type), 'l:type -> 'm:type)
// Info 4164: (202-208): Inferred type: ('j:type, 'k:type)
// Info 4164: (203-204): Inferred type: 'j:type
// Info 4164: (206-207): Inferred type: 'k:type
// Info 4164: (229-265): Inferred type: tfun(('p:type, 'q:type), ('p:type, 'q:type))
// Info 4164: (238-244): Inferred type: ('n:type, 'o:type)
// Info 4164: (239-240): Inferred type: 'n:type
// Info 4164: (242-243): Inferred type: 'o:type
// Info 4164: (284-584): Inferred type: () -> ()
// Info 4164: (292-294): Inferred type: ()
// Info 4164: (318-325): Inferred type: void
// Info 4164: (321-325): Inferred type: void
// Info 4164: (340-347): Inferred type: bool
// Info 4164: (343-347): Inferred type: bool
// Info 4164: (357-378): Inferred type: bool
// Info 4164: (357-365): Inferred type: word -> bool
// Info 4164: (357-361): Inferred type: bool
// Info 4164: (366-377): Inferred type: word
// Info 4164: (366-374): Inferred type: bool -> word
// Info 4164: (366-370): Inferred type: bool
// Info 4164: (375-376): Inferred type: bool
// Info 4164: (393-400): Inferred type: word
// Info 4164: (396-400): Inferred type: word
// Info 4164: (414-424): Inferred type: integer
// Info 4164: (417-424): Inferred type: integer
// Info 4164: (438-445): Inferred type: ()
// Info 4164: (441-445): Inferred type: ()
// Info 4164: (460-478): Inferred type: word -> bool
// Info 4164: (463-478): Inferred type: word -> bool
// Info 4164: (463-466): Inferred type: tfun((word, bool), word -> bool)
// Info 4164: (467-471): Inferred type: word
// Info 4164: (473-477): Inferred type: bool
// Info 4164: (488-496): Inferred type: bool
// Info 4164: (488-489): Inferred type: bool
// Info 4164: (492-496): Inferred type: bool
// Info 4164: (492-493): Inferred type: word -> bool
// Info 4164: (494-495): Inferred type: word
// Info 4164: (511-530): Inferred type: (bool, word)
// Info 4164: (514-530): Inferred type: (bool, word)
// Info 4164: (514-518): Inferred type: tfun((bool, word), (bool, word))
// Info 4164: (519-523): Inferred type: bool
// Info 4164: (525-529): Inferred type: word
// Info 4164: (540-553): Inferred type: bool
// Info 4164: (540-550): Inferred type: (bool, word) -> bool
// Info 4164: (540-544): Inferred type: ('bc:type, 'bd:type)
// Info 4164: (551-552): Inferred type: (bool, word)
// Info 4164: (563-577): Inferred type: word
// Info 4164: (563-574): Inferred type: (bool, word) -> word
// Info 4164: (563-567): Inferred type: ('bh:type, 'bi:type)
// Info 4164: (575-576): Inferred type: (bool, word)
