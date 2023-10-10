pragma experimental solidity;

type word = __builtin("word");

type T = word;

class Self: C {
    function f(self: Self) -> word;
}

instantiation T: C {
    function f(self: T) -> word {}
}

instantiation T: D {
    function f(self: T) -> word {}
}

class Self: D {
    function f(self: Self) -> word;
}
// ----
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (31-61): Inferred type: word
// Info 4164: (63-77): Inferred type: T:(type, C, D)
// Info 4164: (72-76): Inferred type: word
// Info 4164: (79-132): Inferred type: C
// Info 4164: (85-89): Inferred type: 'bb:(type, C)
// Info 4164: (99-130): Inferred type: 'bb:(type, C) -> word
// Info 4164: (109-121): Inferred type: 'bb:(type, C)
// Info 4164: (110-120): Inferred type: 'bb:(type, C)
// Info 4164: (116-120): Inferred type: 'bb:(type, C)
// Info 4164: (125-129): Inferred type: word
// Info 4164: (134-191): Inferred type: void
// Info 4164: (159-189): Inferred type: T -> word
// Info 4164: (169-178): Inferred type: T
// Info 4164: (170-177): Inferred type: T
// Info 4164: (176-177): Inferred type: T
// Info 4164: (182-186): Inferred type: word
// Info 4164: (193-250): Inferred type: void
// Info 4164: (218-248): Inferred type: T -> word
// Info 4164: (228-237): Inferred type: T
// Info 4164: (229-236): Inferred type: T
// Info 4164: (235-236): Inferred type: T
// Info 4164: (241-245): Inferred type: word
// Info 4164: (252-305): Inferred type: D
// Info 4164: (258-262): Inferred type: 'bi:(type, D)
// Info 4164: (272-303): Inferred type: 'bi:(type, D) -> word
// Info 4164: (282-294): Inferred type: 'bi:(type, D)
// Info 4164: (283-293): Inferred type: 'bi:(type, D)
// Info 4164: (289-293): Inferred type: 'bi:(type, D)
// Info 4164: (298-302): Inferred type: word
