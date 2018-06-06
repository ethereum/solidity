contract c {
    event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d, uint indexed e) anonymous;
}
// ----
// TypeError: (17-117): More than 4 indexed arguments for anonymous event.
