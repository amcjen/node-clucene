var path = require('path');
var fs = require('fs');
var wrench = require('wrench');
var async = require('async');

var cl = require('../clucene').CLucene;
var clucene = new cl.Lucene();

var indexPath = './heavy.index';

var testJson = JSON.parse(fs.readFileSync("./test/facebook.json"));

if (path.existsSync(indexPath)) {
    wrench.rmdirSyncRecursive(indexPath);
}

console.log("Check start size"); 
setTimeout(function() { doTest(); }, 10000);
function doTest() {
	var doc = new cl.Document();
	var ctr = 0;
	function nextStep() { ctr++; runAgain(); }
	function runAgain() {
		if (ctr > 100000) {
			console.log("All done check the ram");
			delete doc;
			doc = null;
			clucene.closeWriter();
			return setTimeout(function() {}, 360000);
		}
		process.nextTick(function() {
			doc.clear();
			if (ctr % 1000 == 0) console.log("Adding " + ctr);
	    	testJson.newField = ctr;
	    	doc.addField("json", JSON.stringify(testJson), cl.STORE_YES|cl.INDEX_TOKENIZED);
	    	doc.addField("baseId", String(ctr), cl.STORE_YES|cl.INDEX_UNTOKENIZED);
	    	var docId = "id" + ctr;
	    	clucene.addDocument(docId, doc, indexPath, nextStep);
		});
	}
	runAgain();
}

process.stdin.on("data", function(data) {
	if (data == "end") process.exit(0);
	console.log(data);
});