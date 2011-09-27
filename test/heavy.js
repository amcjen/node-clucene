var path = require('path');
var fs = require('fs');
var wrench = require('wrench');

var cl = require('../clucene').CLucene;
var clucene = new cl.Lucene();

var indexPath = './heavy.index';

var testJson = JSON.parse(fs.readFileSync("./test/facebook.json"));

for (var i = 0; i < 100000; ++i) {
	console.log("Adding " + i);
	var doc = new cl.Document();
	testJson.newField = i;
	doc.addField("json", JSON.stringify(testJson), cl.STORE_YES|cl.INDEX_TOKENIZED);
	doc.addField("baseId", String(i), cl.STORE_YES|cl.INDEX_UNTOKENIZED);

	var docId = "id" + i;
	clucene.addDocument(docId, doc, indexPath, function() { });
}

setTimeout(function() {
	console.log("It's all done, go check.");
}, 10000);
