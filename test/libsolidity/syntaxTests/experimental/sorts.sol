pragma experimental solidity;

type word = __builtin("word");
type uint = word;

class Self: Number {}

instantiation uint: Number {}

function f(a: T: Number) {}

function test() {
    let x: uint: Number;
    f(x);
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-61): Inferred type: word
// Info 4164: (62-79): Inferred type: uint:(type, Number)
// Info 4164: (74-78): Inferred type: word
// Info 4164: (81-102): Inferred type: Number
// Info 4164: (87-91): Inferred type: 'v:(type, Number)
// Info 4164: (104-133): Inferred type: void
// Info 4164: (135-162): Inferred type: 'z:(type, Number) -> ()
// Info 4164: (145-159): Inferred type: 'z:(type, Number)
// Info 4164: (146-158): Inferred type: 'z:(type, Number)
// Info 4164: (149-158): Inferred type: 'z:(type, Number)
// Info 4164: (149-150): Inferred type: 'z:(type, Number)
// Info 4164: (152-158): Inferred type: 'z:(type, Number):(type, Number)
// Info 4164: (164-218): Inferred type: () -> ()
// Info 4164: (177-179): Inferred type: ()
// Info 4164: (190-205): Inferred type: uint
// Info 4164: (193-205): Inferred type: uint
// Info 4164: (193-197): Inferred type: uint
// Info 4164: (199-205): Inferred type: uint:(type, Number)
// Info 4164: (211-215): Inferred type: ()
// Info 4164: (211-212): Inferred type: uint -> ()
// Info 4164: (213-214): Inferred type: uint
