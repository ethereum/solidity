{
    { switch 42 case 1 {} }
    { switch 42 case 1 {} case 2 {} }
    { switch 42 case 1 {} default {} }
    { switch 42 case 1 {} case 2 {} default {} }
    { switch mul(1, 2) case 1 {} case 2 {} default {} }
    { function f() -> x {} switch f() case 1 {} case 2 {} default {} }
}
