

function Caption(element, picid, caption) {
  this.element = element;
  this.picid = picid;
  element.value = caption; // objects in Javascript are assigned by reference, so this works
  element.addEventListener("change", this, false);
}

Caption.prototype.handleEvent = function(e) {
  if (e.type === "change") {
    this.update(this.element.value);
  }
}

Caption.prototype.change = function(value) {
  this.data = value;
  this.element.value = value;
}

Caption.prototype.update = function(caption) {
  makeCaptionPostRequest(this.picid, caption, function() {
    console.log('POST successful.');
  });
}

function makeCaptionPostRequest(picid, caption, cb) {
  var data = {
    'id': picid,
    'caption': caption
  };

  qwest.post('/sksnf4gf1or/pa4/pic/caption', data, {
    dataType: 'json',
    responseType: 'json'
  }).then(function(xhr, resp) {
    cb(resp);
  });
}

function makeCaptionRequest(picid, cb) {
  qwest.get('/sksnf4gf1or/pa4/pic/caption?id=' + picid)
    .then(function(xhr, resp) {
      cb(resp);
    });
}

// caption.js bottom function
//query data store for latest fetch data
function initCaption(picid) {
  var caption = document.getElementById("caption");
  var captionBinding = new Caption(caption, picid);

  makeCaptionRequest(picid, function(resp) {
    captionBinding.change(resp['caption']);
  });

  setInterval(function() {
   makeCaptionRequest(picid, function(resp) {
      captionBinding.change(resp['caption']);
    });
  }, 7000);
}
