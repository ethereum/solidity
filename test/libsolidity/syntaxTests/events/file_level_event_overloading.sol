event E();
event E(uint);
event E(uint, string indexed, bytes, bool);
event E(int, int, int) anonymous;

function f() {
    emit E();
    emit E(1);
    emit E(1, "abc", "abc", true);
    emit E(1, 2, 3);
}
