interface I {
    modifier m { _; }
    modifier mu;
    modifier mv virtual { _; }
    modifier muv virtual;
}
// ----
// TypeError 6408: (18-35='modifier m { _; }'): Modifiers cannot be defined or declared in interfaces.
// TypeError 6408: (40-52='modifier mu;'): Modifiers cannot be defined or declared in interfaces.
// TypeError 8063: (40-52='modifier mu;'): Modifiers without implementation must be marked virtual.
// TypeError 6408: (57-83='modifier mv virtual { _; }'): Modifiers cannot be defined or declared in interfaces.
// TypeError 6408: (88-109='modifier muv virtual;'): Modifiers cannot be defined or declared in interfaces.
