pragma experimental solidity;

type word = __builtin("word");
type bool = __builtin("bool");
type unit = __builtin("unit");

type wordOrBool = word | bool;
type wordAndBool = (word, bool);

function g() -> () {}

function f() -> () {
    let ptr1: unit -> unit = g;
    let ptr2: () -> () = g;

    // FIXME: Does not unify:
    //let ptr3: () -> () | () -> () = g;
    //let ptr4: unit -> unit | () -> () = g;
    //let ptr5: word | () -> () = g;
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-61): Inferred type: word
// Info 4164: (62-92): Inferred type: bool
// Info 4164: (93-123): Inferred type: ()
// Info 4164: (125-155): Inferred type: wordOrBool
// Info 4164: (143-154): Inferred type: sum(word, bool)
// Info 4164: (143-147): Inferred type: word
// Info 4164: (150-154): Inferred type: bool
// Info 4164: (156-188): Inferred type: wordAndBool
// Info 4164: (175-187): Inferred type: (word, bool)
// Info 4164: (176-180): Inferred type: word
// Info 4164: (182-186): Inferred type: bool
// Info 4164: (190-211): Inferred type: () -> ()
// Info 4164: (200-202): Inferred type: ()
// Info 4164: (206-208): Inferred type: ()
// Info 4164: (213-449): Inferred type: () -> ()
// Info 4164: (223-225): Inferred type: ()
// Info 4164: (229-231): Inferred type: ()
// Info 4164: (242-260): Inferred type: () -> ()
// Info 4164: (248-260): Inferred type: () -> ()
// Info 4164: (248-252): Inferred type: ()
// Info 4164: (256-260): Inferred type: ()
// Info 4164: (263-264): Inferred type: () -> ()
// Info 4164: (274-288): Inferred type: () -> ()
// Info 4164: (280-288): Inferred type: () -> ()
// Info 4164: (280-282): Inferred type: ()
// Info 4164: (286-288): Inferred type: ()
// Info 4164: (291-292): Inferred type: () -> ()
