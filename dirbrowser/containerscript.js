// This script saves the location details when a mouse button is
// clicked. This is for replacing data provided by QWebElement on a
// right-click if/when we switch to QT WebEngine which does not have
// an equivalent service.  This is always installed but only used if
// upplay is built with webengine (not the case and not possible for
// now)
var locDetails = '';
function saveLoc(ev) 
{
    el = ev.target;
    locDetails = '';
    while (el && el.attributes && !el.attributes.getNamedItem("objid")) {
        // console.log('el: ' + el);
        el = el.parentNode;
    }
    
    objid = el.attributes.getNamedItem("objid");
    if (objid) {
        objidvalue = objid.value;
    } else {
        objidvalue = "";
    }
    objidvalue = el.attributes.getNamedItem("objid").value;
    objidx = el.attributes.getNamedItem("objidx");
    if (objidx) {
        objidxvalue = objidx.value;
    } else {
        objidxvalue = "";
    }
    otype = el.attributes.getNamedItem("class");
    if (otype) {
        otypevalue = otype.value
    } else {
        otypevalue = "";
    }
    if (el && el.attributes) {
        locDetails = 
            'objid = ' + objidvalue + '\n' +
            'title = ' + (el.textContent || el.innerText || "") + '\n' +
            'objidx = ' + objidxvalue + '\n' +
            'otype = ' + otypevalue + '\n';
    }
    //console.log(locDetails);
}
