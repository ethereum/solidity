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
// TypeError 3464: (87-98): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// Warning 5740: (146-151): Unreachable code.
// Warning 5740: (169-174): Unreachable code.
// TypeError 3464: (223-234): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// Warning 5740: (316-321): Unreachable code.
// TypeError 3464: (440-451): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (654-665): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (871-882): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// Warning 5740: (933-938): Unreachable code.
