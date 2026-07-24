/*
 * Version global definitions
 */

var XCALCVER = "3.0.4";
var XCALCDATE = "May 2012";
var XCALCFULLDATE = "2 May 2012";
var EMAIL = "bernt.ribbum@gmail.com";
var WEB = "http://www.tordivel.no/xcalc";

function XCALCVersion() {
	return XCALCVER;
}

function WriteVersion() {
	document.write(XCALCVER);
}

function WriteDate() {
	document.write(XCALCDATE);
}

function WriteMail(subject,text) {
	if (!subject) subject = "(help page)";
	if (!text) text = EMAIL;
	document.write('<a href="mailto:'+EMAIL);
	document.write('?subject=XCALC '+XCALCVER+' '+subject);
	document.write('">'+text+'</a>');
}

function WriteWeb() {
	document.write('<a href="');
	document.write(WEB);
	document.write('">');
	document.write(WEB);
	document.write('</a>');
}

