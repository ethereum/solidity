library L{ constructor() { L.x; } }
// ----
// TypeError 7634: (11-33='constructor() { L.x; }'): Constructor cannot be defined in libraries.
// TypeError 9582: (27-30='L.x'): Member "x" not found or not visible after argument-dependent lookup in type(library L).
