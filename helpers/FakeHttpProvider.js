var chai = require('chai');
var assert = require('assert');
var utils = require('../../lib/utils/utils');

var getResponseStub = function () {
    return {
        jsonrpc: '2.0',
        id: 1,
        result: 0
    };
};

var FakeHttpProvider = function () {
    this.response = getResponseStub();
    this.error = null;
    this.validation = null;
};

FakeHttpProvider.prototype.send = function (payload) {
    assert.equal(utils.isArray(payload) || utils.isObject(payload), true);
    // TODO: validate jsonrpc request
    if (this.error) {
        throw this.error;
    } 
    if (this.validation) {
        // imitate plain json object
        this.validation(JSON.parse(JSON.stringify(payload)));
    }
    return this.response;
};

FakeHttpProvider.prototype.sendAsync = function (payload, callback) {
    assert.equal(utils.isArray(payload) || utils.isObject(payload), true);
    assert.equal(utils.isFunction(callback), true);
    if (this.validation) {
        // imitate plain json object
        this.validation(JSON.parse(JSON.stringify(payload)), callback);
    }
    callback(this.error, this.response);
};

FakeHttpProvider.prototype.injectResponse = function (response) {
    this.response = response;
};

FakeHttpProvider.prototype.injectResult = function (result) {
    this.response = getResponseStub();
    this.response.result = result;
};

FakeHttpProvider.prototype.injectBatchResults = function (results) {
    this.response = results.map(function (r) {
        var response = getResponseStub();
        response.result = r;
        return response;
    }); 
};

FakeHttpProvider.prototype.injectError = function (error) {
    this.error = error;
};

FakeHttpProvider.prototype.injectValidation = function (callback) {
    this.validation = callback;
};

module.exports = FakeHttpProvider;

