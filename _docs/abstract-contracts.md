---
layout: docs
title: Abstract Contracts
permalink: /docs/abstract-contracts/
---

Contract functions can lack an implementation as in the following example (note that the function declaration header is terminated by `;`).
{% highlight javascript %}
contract feline {
  function utterance() returns (bytes32);
}
{% endhighlight %}
Such contracts cannot be compiled (even if they contain implemented functions alongside non-implemented functions), but they can be used as base contracts:
{% highlight javascript %}
contract Cat is feline {
  function utterance() returns (bytes32) { return "miaow"; }
}
{% endhighlight %}
If a contract inherits from an abstract contract and does not implement all non-implemented functions by overriding, it will itself be abstract.
