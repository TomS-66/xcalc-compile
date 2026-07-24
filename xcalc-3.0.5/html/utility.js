/*
 * Hacked from an Apple example by BR Jan 08
 */

function getStyleObject(objectId) {
    // any useful browser supports the W3C DOM now
	return document.getElementById(objectId).style;
} // getStyleObject

function changeObjectVisibility(objectId, newVisibility) {
    // get a reference to the cross-browser style object and make sure the object exists
    var styleObject = getStyleObject(objectId);
    if(styleObject) {
		styleObject.visibility = newVisibility;
		return true;
    } else {
		// we couldn't find the object, so we can't change its visibility
		return false;
    }
} // changeObjectVisibility

function moveObject(objectId, newXCoordinate, newYCoordinate) {
    // get a reference to the cross-browser style object and make sure the object exists
    var styleObject = getStyleObject(objectId);
    if(styleObject) {
		styleObject.left = newXCoordinate;
		styleObject.top = newYCoordinate;
		return true;
    } else {
		// we couldn't find the object, so we can't very well move it
		return false;
    }
} // moveObject

