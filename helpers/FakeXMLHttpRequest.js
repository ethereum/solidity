var chai = require('chai');
var assert = chai.assert;

var FakeXMLHttpRequest = function () {
    this.responseText = "{}";
    this.readyState = 4;
    this.onreadystatechange = null;
    this.async = false;
};

FakeXMLHttpRequest.prototype.open = function (method, host, async) {
    assert.equal(method, 'POST');
    assert.notEqual(host, null);
    assert.equal(async === false || async === true, true);
    this.async = async;
};

FakeXMLHttpRequest.prototype.send = function (payload) {
    assert.equal(typeof payload, 'string');
    if (this.async) {
        assert.equal(typeof this.onreadystatechange, 'function');
        this.onreadystatechange();
        return;
    }
    return this.responseText;
};

module.exports = { 
    XMLHttpRequest: FakeXMLHttpRequest
};

