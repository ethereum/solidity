contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        do {
            break;
            c = s;
        } while(false);
    }
    function g() internal view returns (S storage c) {
        do {
            if (s.f) {
                continue;
                c = s;
            }
            else {
            }
        } while(false);
    }
    function h() internal view returns (S storage c) {
        do {
            if (s.f) {
                break;
            }
            else {
                c = s;
            }
        } while(false);
    }
    function i() internal view returns (S storage c) {
        do {
            if (s.f) {
                continue;
            }
            else {
                c = s;
            }
        } while(false);
    }
    function j() internal view returns (S storage c) {
        do {
            continue;
            c = s;
        } while(false);
    }
}
// ----
// TypeError: (87-98): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (223-234): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (440-451): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (654-665): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (871-882): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
