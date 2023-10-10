pragma experimental solidity;

type word = __builtin("word");

function f(x: word) -> word {
    g(x);
}

function g(x: word) -> word {
    f(x);
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-61): Inferred type: word
// Info 4164: (63-104): Inferred type: word -> word
// Info 4164: (73-82): Inferred type: word
// Info 4164: (74-81): Inferred type: word
// Info 4164: (77-81): Inferred type: word
// Info 4164: (86-90): Inferred type: word
// Info 4164: (97-101): Inferred type: word
// Info 4164: (97-98): Inferred type: word -> word
// Info 4164: (99-100): Inferred type: word
// Info 4164: (106-147): Inferred type: word -> word
// Info 4164: (116-125): Inferred type: word
// Info 4164: (117-124): Inferred type: word
// Info 4164: (120-124): Inferred type: word
// Info 4164: (129-133): Inferred type: word
// Info 4164: (140-144): Inferred type: word
// Info 4164: (140-141): Inferred type: word -> word
// Info 4164: (142-143): Inferred type: word
