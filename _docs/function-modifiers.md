---
layout: docs
title: Function Modifiers
permalink: /docs/function-modifiers/
---

Modifiers can be used to easily change the behaviour of functions, for example
to automatically check a condition prior to executing the function. They are
inheritable properties of contracts and may be overridden by derived contracts.

{% highlight javascript %}
contract owned {
  function owned() { owner = msg.sender; }
  address owner;

  // This contract only defines a modifier but does not use
  // it - it will be used in derived contracts.
  // The function body is inserted where the special symbol
  // "_" in the definition of a modifier appears.
  // This means that if the owner calls this function, the
  // function is executed and otherwise, an exception is
  // thrown.
  modifier onlyowner { if (msg.sender != owner) throw; _ }
}
contract mortal is owned {
  // This contract inherits the "onlyowner"-modifier from
  // "owned" and applies it to the "close"-function, which
  // causes that calls to "close" only have an effect if
  // they are made by the stored owner.
  function close() onlyowner {
    selfdestruct(owner);
  }
}
contract priced {
  // Modifiers can receive arguments:
  modifier costs(uint price) { if (msg.value >= price) _ }
}
contract Register is priced, owned {
  mapping (address => bool) registeredAddresses;
  uint price;
  function Register(uint initialPrice) { price = initialPrice; }
  function register() costs(price) {
    registeredAddresses[msg.sender] = true;
  }
  function changePrice(uint _price) onlyowner {
    price = _price;
  }
}
{% endhighlight %}

Multiple modifiers can be applied to a function by specifying them in a
whitespace-separated list and will be evaluated in order. Explicit returns from
a modifier or function body immediately leave the whole function, while control
flow reaching the end of a function or modifier body continues after the "_" in
the preceding modifier. Arbitrary expressions are allowed for modifier
arguments and in this context, all symbols visible from the function are
visible in the modifier. Symbols introduced in the modifier are not visible in
the function (as they might change by overriding).
